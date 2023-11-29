/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_eventdev.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>

#include "link.h"
#include "stage.h"
#include "vswitch.h"

#define DEFAULT_PKT_BURST (32)

static struct vswitch_config *config = NULL;

int
fwd_pkts(uint16_t link_id, uint16_t peer_link_id)
{
	struct rte_mbuf *mbufs[DEFAULT_PKT_BURST];
	uint16_t rc;

	rc = rte_eth_rx_burst(link_id, 0, mbufs, DEFAULT_PKT_BURST);
	if (rc > 0) {
		rte_eth_tx_burst(peer_link_id, 0, mbufs, rc);
	}

	return rc;
}

static int launch_worker(__attribute__((unused)) void *arg) {
	uint16_t core_id = rte_lcore_id();

	RTE_LOG(INFO, USER1, "Worker %u starting\n", core_id);

	while(1) {
		link_map_walk(fwd_pkts);
	}

	RTE_LOG(INFO, USER1, "Worker %u stopping\n", core_id);

	return 0;
}

int
vswitch_init()
{
	int num_eventdev = 0;
	uint16_t core_id;
	int rc = -EINVAL;

	if (!config) {
		config = rte_malloc(NULL, sizeof(struct vswitch_config), 0);
		if (!config) {
			rc = -rte_errno;
			goto err;
		}
	}

	num_eventdev = rte_event_dev_count();
	if (num_eventdev < 1) {
		rc = -ENOENT;
		goto err;
	}

	// TODO: Pick eventdev based on configuration instead
	memset(config, 0, sizeof(*config));
	config->ev_id = 0;
	rc = rte_event_dev_info_get(config->ev_id, &config->ev_info);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		config->lcores[core_id].core_id = core_id;
		config->lcores[core_id].enabled = 0;
		config->lcores[core_id].ev_port_id = core_id;
		config->lcores[core_id].ev_queue_needed = 0;
		config->lcores[core_id].ev_queue_id = EV_QUEUE_ID_INVALID;
	}

	return 0;

err:
	return rc;
}

int
vswitch_quit()
{
	rte_free(config);
	return 0;
}

struct vswitch_config *
vswitch_config_get()
{
	return config;
}

static int
stage_info_get(__rte_unused struct stage_config *stage_config, __rte_unused void *data)
{
	struct lcore_params *lcore;
	uint16_t core_id;

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		if (stage_config->coremask & (1UL << core_id)) {
			lcore = &config->lcores[core_id];
			lcore->enabled = 1;
			lcore->ev_port_config.dequeue_depth = 128;
			lcore->ev_port_config.enqueue_depth = 128;
			lcore->ev_port_config.new_event_threshold = 4096;
			config->nb_ports++;
			switch (stage_config->queue.type) {
			case STAGE_TYPE_WORKER:
				lcore->ev_queue_needed = 1;
				lcore->ev_queue_id = stage_config->queue.worker.in;
				break;
			case STAGE_TYPE_TX:
				lcore->ev_queue_needed = 1;
				lcore->ev_queue_id = stage_config->queue.worker.in;
				break;
			case STAGE_TYPE_RX:
			default:
				lcore->ev_queue_needed = 0;
				break;
			}

			if (lcore->ev_queue_needed) {
				config->nb_queues++;
			}
		}
	}

	return 0;
}

static int
stage_configure(__rte_unused struct stage_config *stage_config, __rte_unused void *data)
{
	int rc = 0;

	if (stage_config->queue.type == STAGE_TYPE_WORKER) {
		rc = rte_event_queue_setup(config->ev_id,
					   stage_config->queue.worker.in,
					   &stage_config->queue.worker.ev_queue_config);
	} else if (stage_config->queue.type == STAGE_TYPE_TX) {
		rc = rte_event_queue_setup(config->ev_id,
					   stage_config->queue.tx.in,
					   &stage_config->queue.tx.ev_queue_config);
	}

	return rc;
}

int
vswitch_start()
{
	struct rte_event_dev_config ev_config;
	struct lcore_params *lcore;
	uint16_t core_id;
	int rc = -EINVAL;

	// Start all links
	link_start();

	// Fetch configuration from all stages
	stage_config_walk(stage_info_get, config);

	memset(&ev_config, 0, sizeof(ev_config));
	ev_config.nb_event_queues = config->nb_queues;
	ev_config.nb_event_ports = config->nb_ports;
	ev_config.nb_events_limit  = config->ev_info.max_num_events;
	ev_config.nb_event_queue_flows = 1024;
	ev_config.nb_event_port_dequeue_depth = config->ev_info.max_event_port_dequeue_depth;
	ev_config.nb_event_port_enqueue_depth = config->ev_info.max_event_port_enqueue_depth;
	rc = rte_event_dev_configure(config->ev_id, &ev_config);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	// Configure all stages
	stage_config_walk(stage_configure, config);

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		lcore = &config->lcores[core_id];
		if (!lcore->enabled)
			continue;

		rc = rte_event_port_setup(config->ev_id,
					  lcore->ev_port_id,
					  &lcore->ev_port_config);
		if (rc < 0) {
			rc = -rte_errno;
			goto err;
		}

		rc = rte_event_port_link(config->ev_id,
					  lcore->ev_port_id,
					  &lcore->ev_queue_id,
					  NULL,
					  1);
		if (rc < 0) {
			rc = -rte_errno;
			goto err;
		}
	}


	rc = rte_event_dev_start(config->ev_id);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	RTE_LCORE_FOREACH_WORKER(core_id) {
		rte_eal_remote_launch(launch_worker, NULL, core_id);
	}

	return 0;

err:
	return rc;
}
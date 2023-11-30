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
#include <rte_service.h>

#include "link.h"
#include "stage.h"
#include "vswitch.h"

#define DEFAULT_PKT_BURST (32)
#define DEFAULT_EVENT_BURST (32)

static struct vswitch_config *config = NULL;

int
produce_pkts(uint16_t link_id, uint16_t peer_link_id, void *arg)
{
	struct lcore_params *lcore = (struct lcore_params*) arg;
	struct rte_event events[DEFAULT_EVENT_BURST];
	struct rte_mbuf *mbufs[DEFAULT_PKT_BURST];
	uint16_t nb_rx, nb_tx;
	int i;

	nb_rx = rte_eth_rx_burst(link_id,
				 0,
				 mbufs,
				 DEFAULT_PKT_BURST);
	for (i = 0; i < nb_rx; i++) {
		events[i].flow_id = mbufs[i]->hash.rss;
		events[i].op = RTE_EVENT_OP_NEW;
		events[i].sched_type = lcore->type;
		events[i].queue_id = lcore->ev_out_queue;
		events[i].event_type = RTE_EVENT_TYPE_ETHDEV;
		events[i].sub_event_type = peer_link_id;
		events[i].priority = RTE_EVENT_DEV_PRIORITY_NORMAL;
		events[i].mbuf = mbufs[i];
	}

	nb_tx = rte_event_enqueue_burst(lcore->ev_id,
					lcore->ev_port_id,
					events,
					nb_rx);
	if (nb_tx != nb_rx) {
		RTE_LOG(INFO, USER1, "Core (%u) Received (%u). transmitted (%u)",
			lcore->core_id, nb_rx, nb_tx);
		for(i = nb_tx; i < nb_rx; i++)
			rte_pktmbuf_free(mbufs[i]);
		return -EIO;
	}

	return 0;
}

static int
launch_rx(void *arg)
{
	struct lcore_params *lcore = (struct lcore_params*) arg;
	uint16_t core_id = rte_lcore_id();

	RTE_LOG(INFO, USER1, "RX producer %u starting\n", core_id);

	while(1) {
		link_map_walk(produce_pkts, lcore);
	}

	RTE_LOG(INFO, USER1, "RX producer %u stopping\n", core_id);

	return 0;
}

static int
launch_worker(void *arg)
{
	struct lcore_params *lcore = (struct lcore_params*) arg;
	struct rte_event events[DEFAULT_EVENT_BURST];
	uint16_t core_id = rte_lcore_id();
	uint16_t nb_rx, nb_tx;
	int i, timeout = 0;

	RTE_LOG(INFO, USER1, "Worker %u starting\n", core_id);

	while(1) {
		nb_rx = rte_event_dequeue_burst(lcore->ev_id,
						lcore->ev_port_id,
						events,
						DEFAULT_EVENT_BURST,
						timeout);

		for (i = 0; i < nb_rx; i++) {
			events[i].op = RTE_EVENT_OP_FORWARD;
			events[i].queue_id = lcore->ev_out_queue;
		}

		nb_tx = rte_event_enqueue_burst(lcore->ev_id,
						lcore->ev_port_id,
						events,
						nb_rx);
		
		if (nb_tx != nb_rx) {
			RTE_LOG(INFO, USER1, "Core (%u) Received (%u). transmitted (%u)",
				core_id, nb_rx, nb_tx);
		}
	}

	RTE_LOG(INFO, USER1, "Worker %u stopping\n", core_id);

	return 0;
}

static int
launch_tx(void *arg)
{
	struct lcore_params *lcore = (struct lcore_params*) arg;
	struct rte_event events[DEFAULT_EVENT_BURST];
	uint16_t core_id = rte_lcore_id();
	int i, timeout = 0;
	uint16_t nb_rx;

	RTE_LOG(INFO, USER1, "TX consumer %u starting\n", core_id);

	while(1) {
		nb_rx = rte_event_dequeue_burst(lcore->ev_id,
						lcore->ev_port_id,
						events,
						DEFAULT_EVENT_BURST,
						timeout);

		for (i = 0; i < nb_rx; i++) {
			rte_eth_tx_burst(events[i].sub_event_type,
					 0,
					 &events[i].mbuf,
					 1);
		}
	}

	RTE_LOG(INFO, USER1, "TX consumer %u stopping\n", core_id);

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
		config->lcores[core_id].ev_id = config->ev_id;
		// config->lcores[core_id].ev_port_id = core_id;
		config->lcores[core_id].ev_in_queue_needed = 0;
		config->lcores[core_id].ev_in_queue = EV_QUEUE_ID_INVALID;
		config->lcores[core_id].ev_out_queue_needed = 0;
		config->lcores[core_id].ev_out_queue = EV_QUEUE_ID_INVALID;
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
			lcore->type = stage_config->queue.type;
			lcore->ev_port_config.dequeue_depth = 128;
			lcore->ev_port_config.enqueue_depth = 128;
			lcore->ev_port_config.new_event_threshold = 4096;
			lcore->ev_port_id = config->nb_ports++;
			switch (stage_config->queue.type) {
			case STAGE_TYPE_RX:
				lcore->ev_out_queue_needed = 1;
				lcore->ev_out_queue = stage_config->queue.out;
				break;
			case STAGE_TYPE_WORKER:
				lcore->ev_in_queue_needed = 1;
				lcore->ev_in_queue = stage_config->queue.in;
				lcore->ev_out_queue_needed = 1;
				lcore->ev_out_queue = stage_config->queue.out;
				break;
			case STAGE_TYPE_TX:
				lcore->ev_in_queue_needed = 1;
				lcore->ev_in_queue = stage_config->queue.in;
				break;
			default:
				break;
			}
		}
	}

	if (stage_config->queue.type == STAGE_TYPE_WORKER ||
	    stage_config->queue.type == STAGE_TYPE_TX)
		config->nb_queues++;

	return 0;
}

static int
stage_configure(__rte_unused struct stage_config *stage_config, __rte_unused void *data)
{
	int rc = 0;

	if (stage_config->queue.type == STAGE_TYPE_WORKER ||
	    stage_config->queue.type == STAGE_TYPE_TX) {
		rc = rte_event_queue_setup(config->ev_id,
					   stage_config->queue.in,
					   &stage_config->queue.ev_queue_config);
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

		RTE_LOG(INFO, USER1, "Lcore %u config: "
			"type: %u "
			"ev_id: %u, ev_port_id: %u "
			"ev_in_queue: %u ev_out_queue: %u "
			"\n",
			core_id, lcore->type,
			lcore->ev_id, lcore->ev_port_id,
			lcore->ev_in_queue, lcore->ev_out_queue);

		rc = rte_event_port_setup(config->ev_id,
					  lcore->ev_port_id,
					  &lcore->ev_port_config);
		if (rc < 0) {
			rc = -rte_errno;
			goto err;
		}

		if (!lcore->ev_in_queue_needed)
			continue;

		rc = rte_event_port_link(config->ev_id,
					  lcore->ev_port_id,
					  &lcore->ev_in_queue,
					  NULL,
					  1);
		if (rc < 0) {
			rc = -rte_errno;
			goto err;
		}
	}

	rc = rte_event_dev_service_id_get(config->ev_id, &config->ev_service_id);
	if (rc != -ESRCH && rc != 0) {
		goto err;
	}
	rte_service_runstate_set(config->ev_service_id, 1);
	rte_service_set_runstate_mapped_check(config->ev_service_id, 0);

	rc = rte_event_dev_start(config->ev_id);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	RTE_LCORE_FOREACH_WORKER(core_id) {
		lcore = &config->lcores[core_id];
		switch(lcore->type) {
		case STAGE_TYPE_RX:
			rte_eal_remote_launch(launch_rx, lcore, core_id);
			break;
		case STAGE_TYPE_WORKER:
			rte_eal_remote_launch(launch_worker, lcore, core_id);
			break;
		case STAGE_TYPE_TX:
			rte_eal_remote_launch(launch_tx, lcore, core_id);
			break;
		default:
			break;
		}
	}

	return 0;

err:
	return rc;
}
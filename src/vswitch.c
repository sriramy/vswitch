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
#include <rte_node_eth_api.h>
#include <rte_service.h>

#include "link.h"
#include "stage.h"
#include "vswitch.h"
#include "node/eventdev_dispatcher.h"
#include "node/eventdev_rx.h"
#include "node/eventdev_tx.h"

#define DEFAULT_PKT_BURST (32)
#define DEFAULT_EVENT_BURST (32)

static struct vswitch_config *config = NULL;

int
produce_pkts(uint16_t link_id, uint16_t queue_id, void *arg)
{
	struct lcore_params *lcore = (struct lcore_params*) arg;
	struct rte_event events[DEFAULT_EVENT_BURST];
	struct rte_mbuf *mbufs[DEFAULT_PKT_BURST];
	uint16_t nb_rx = 0, nb_tx = 0;
	int i;

	nb_rx = rte_eth_rx_burst(link_id,
				 queue_id,
				 mbufs,
				 DEFAULT_PKT_BURST);
	for (i = 0; i < nb_rx; i++) {
		events[i].flow_id = mbufs[i]->hash.rss;
		events[i].op = RTE_EVENT_OP_NEW;
		events[i].sched_type = lcore->ev_out_queue_sched_type;
		events[i].queue_id = lcore->ev_out_queue;
		events[i].event_type = RTE_EVENT_TYPE_ETHDEV;
		events[i].sub_event_type = 0;
		events[i].priority = RTE_EVENT_DEV_PRIORITY_NORMAL;
		events[i].mbuf = mbufs[i];
	}

	if (nb_rx > 0) {
		nb_tx = rte_event_enqueue_burst(lcore->ev_id,
				lcore->ev_port_id,
				events,
				nb_rx);

		RTE_LOG(DEBUG, USER1, "Core (%u) Received (%u). transmitted (%u)\n",
			lcore->core_id, nb_rx, nb_tx);
	} else {
		rte_pause();
	}

	if (nb_tx != nb_rx) {
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
	int i;

	RTE_LOG(INFO, USER1, "RX producer %u starting\n", core_id);

	while(1) {
		for (i = 0; i < lcore->nb_link_in_queues; i++) {
			produce_pkts(lcore->link_in_queues[i].link_id,
				     lcore->link_in_queues[i].queue_id,
				     arg);
		}
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

		if (nb_rx > 0)
			RTE_LOG(DEBUG, USER1, "Core (%u) Received (%u). transmitted (%u)\n",
				core_id, nb_rx, nb_tx);
		else
			rte_pause();
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
	uint16_t nb_rx, peer_link_id;

	RTE_LOG(INFO, USER1, "TX consumer %u starting\n", core_id);

	while(1) {
		nb_rx = rte_event_dequeue_burst(lcore->ev_id,
						lcore->ev_port_id,
						events,
						DEFAULT_EVENT_BURST,
						timeout);

		for (i = 0; i < nb_rx; i++) {
			if (link_get_peer(events[i].mbuf->port, &peer_link_id))
				rte_eth_tx_burst(peer_link_id,
						0,
						&events[i].mbuf,
						1);
		}

		if (nb_rx > 0)
			RTE_LOG(DEBUG, USER1, "Core (%u) Received (%u)\n", core_id, nb_rx);
		else
			rte_pause();
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
		config->lcores[core_id].nb_link_in_queues = 0;
		config->lcores[core_id].nb_link_out_queues = 0;
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
stage_get_lcore_config(__rte_unused struct stage_config *stage_config, __rte_unused void *data)
{
	struct stage_link_queue_config *qconf;
	struct lcore_params *lcore;
	uint16_t core_id;
	int i;

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		if (stage_config->coremask & (1UL << core_id)) {
			lcore = &config->lcores[core_id];
			lcore->enabled = 1;
			lcore->type = stage_config->type;
			lcore->ev_port_config.dequeue_depth = 128;
			lcore->ev_port_config.enqueue_depth = 128;
			lcore->ev_port_config.new_event_threshold = 4096;
			lcore->ev_port_id = config->nb_ports++;
			switch (stage_config->type) {
			case STAGE_TYPE_RX:
				lcore->ev_out_queue_needed = 1;
				lcore->ev_out_queue_sched_type = stage_config->ev_queue.sched_type_out;
				lcore->ev_out_queue = stage_config->ev_queue.out;
				break;
			case STAGE_TYPE_WORKER:
				lcore->ev_in_queue_needed = 1;
				lcore->ev_in_queue_sched_type = stage_config->ev_queue.sched_type_in;
				lcore->ev_in_queue = stage_config->ev_queue.in;
				lcore->ev_out_queue_needed = 1;
				lcore->ev_out_queue_sched_type = stage_config->ev_queue.sched_type_out;
				lcore->ev_out_queue = stage_config->ev_queue.out;
				break;
			case STAGE_TYPE_TX:
				lcore->ev_in_queue_needed = 1;
				lcore->ev_in_queue_sched_type = stage_config->ev_queue.sched_type_in;
				lcore->ev_in_queue = stage_config->ev_queue.in;
				break;
			default:
				break;
			}

			for (i = 0; i < STAGE_MAX_LINK_QUEUES; i++) {
				qconf = &stage_config->link_in_queue[i];
				if (!qconf->enabled)
					continue;
				lcore->link_in_queues[lcore->nb_link_in_queues].link_id = qconf->link_id;
				lcore->link_in_queues[lcore->nb_link_in_queues].queue_id = qconf->queue_id;
				lcore->nb_link_in_queues++;
			}
			for (i = 0; i < STAGE_MAX_LINK_QUEUES; i++) {
				qconf = &stage_config->link_out_queue[i];
				if (!qconf->enabled)
					continue;
				lcore->link_out_queues[lcore->nb_link_out_queues].link_id = qconf->link_id;
				lcore->link_out_queues[lcore->nb_link_out_queues].queue_id = qconf->queue_id;
				lcore->nb_link_out_queues++;
			}
		}
	}

	if (stage_config->type == STAGE_TYPE_WORKER ||
	    stage_config->type == STAGE_TYPE_TX)
		config->nb_queues++;

	return 0;
}

static int
stage_configure_input_queues(__rte_unused struct stage_config *stage_config, __rte_unused void *data)
{
	int rc = 0;

	if (config->ev_info.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
		stage_config->ev_queue.config_in.event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;

	if (stage_config->type == STAGE_TYPE_WORKER ||
	    stage_config->type == STAGE_TYPE_TX) {
		rc = rte_event_queue_setup(config->ev_id,
					   stage_config->ev_queue.in,
					   &stage_config->ev_queue.config_in);
	}

	return rc;
}

int
vswitch_start()
{
	struct rte_node_ethdev_rx_config rx_config;
	struct rte_node_ethdev_tx_config tx_config;
	struct rte_event_dev_config ev_config;
	char node_name[RTE_NODE_NAMESIZE];
	struct lcore_params *lcore;
	rte_node_t node_id;
	uint16_t core_id;
	int rc = -EINVAL;
	int i;

	// Start all links
	link_start();

	// Fetch all stage per-lcore configuration
	stage_config_walk(stage_get_lcore_config, config);

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

	// Configure all stage input queues
	stage_config_walk(stage_configure_input_queues, config);

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

		if (lcore->ev_in_queue_needed) {
			rc = rte_event_port_link(config->ev_id,
						lcore->ev_port_id,
						&lcore->ev_in_queue,
						NULL,
						1);
			if (rc < 0) {
				rc = -rte_errno;
				goto err;
			}

			snprintf(node_name, sizeof(node_name), 
				"%u-%u", lcore->ev_port_id, lcore->ev_in_queue);
			node_id = eventdev_rx_node_clone(node_name);
			if (node_id == RTE_NODE_ID_INVALID) {
				RTE_LOG(INFO, USER1, "Eventdev rx node (%s) create failed\n", node_name);
				continue;
			}

			rc = eventdev_rx_node_data_add(node_id,
						  lcore->ev_id,
						  lcore->ev_port_id);
			if (rc < 0)
				goto err;

			for (i = 0; i < lcore->nb_link_out_queues; i++) {
				tx_config.link_id = lcore->link_out_queues[i].link_id;
				tx_config.queue_id = lcore->link_out_queues[i].queue_id;
				node_id = rte_node_ethdev_tx_config(&tx_config);
				if (node_id == RTE_NODE_ID_INVALID) {
					RTE_LOG(INFO, USER1, "Ethdev tx node (%u:%u) create failed\n",
						tx_config.link_id, tx_config.queue_id);
					continue;
				}

				eventdev_dispatcher_set_next(
					rte_node_id_to_name(node_id),
					tx_config.link_id,
					tx_config.queue_id);
			}
		}

		if (lcore->ev_out_queue_needed) {
			snprintf(node_name, sizeof(node_name), 
				"%u-%u", lcore->ev_port_id, lcore->ev_out_queue);
			node_id = eventdev_tx_node_clone(node_name);
			if (node_id == RTE_NODE_ID_INVALID) {
				RTE_LOG(INFO, USER1, "Eventdev tx node (%s) create failed\n", node_name);
				continue;
			}

			rc = eventdev_tx_node_data_add(node_id,
						  lcore->ev_id,
						  lcore->ev_port_id,
						  (lcore->ev_in_queue_needed) ? RTE_EVENT_OP_FORWARD : RTE_EVENT_OP_NEW,
						  lcore->ev_out_queue_sched_type,
						  lcore->ev_out_queue,
						  RTE_EVENT_TYPE_ETHDEV,
						  0,
						  RTE_EVENT_DEV_PRIORITY_NORMAL);
			if (rc < 0)
				goto err;

			for (i = 0; i < lcore->nb_link_in_queues; i++) {
				rx_config.link_id = lcore->link_in_queues[i].link_id;
				rx_config.queue_id = lcore->link_in_queues[i].queue_id;
				strncpy(rx_config.next_node,
					rte_node_id_to_name(node_id),
					sizeof(rx_config.next_node));
				rte_node_ethdev_rx_config(&rx_config);
			}
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
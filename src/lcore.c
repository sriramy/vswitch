/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_eventdev.h>
#include <rte_graph.h>
#include <rte_graph_worker.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_node_eth_api.h>

#include "lcore.h"
#include "link.h"
#include "stage.h"
#include "node/eventdev_dispatcher.h"
#include "node/eventdev_rx.h"
#include "node/eventdev_tx.h"
#include "node/forward.h"

void
lcore_init(uint16_t core_id, uint8_t ev_id, struct lcore_params *lcore)
{
        lcore->core_id = core_id;
        lcore->enabled = 0;
        lcore->ev_id = ev_id;
        lcore->ev_port_id = 0;
        lcore->ev_in_queue_needed = 0;
        lcore->ev_in_queue = EV_QUEUE_ID_INVALID;
        lcore->ev_out_queue_needed = 0;
        lcore->ev_out_queue = EV_QUEUE_ID_INVALID;
        lcore->nb_link_in_queues = 0;
        lcore->nb_link_out_queues = 0;

}

int
lcore_config_populate(struct stage_config *stage_config, uint8_t ev_port_id, struct lcore_params *lcore)
{
	struct stage_link_queue_config *qconf;
	int i;

	/* generic config */
	lcore->enabled = 1;
	lcore->type = stage_config->type;
	strncpy(lcore->nodes, stage_config->nodes, STAGE_GRAPH_NODES_MAX_LEN);

	/* event port config */
	lcore->ev_port_config.dequeue_depth = 128;
	lcore->ev_port_config.enqueue_depth = 128;
	lcore->ev_port_config.new_event_threshold = 4096;
	lcore->ev_port_id = ev_port_id;

	/* event queue config */
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
		strncpy(lcore->ev_in_queue_mp_name, stage_config->ev_queue.mp_name, RTE_MEMPOOL_NAMESIZE);
		lcore->ev_out_queue_needed = 1;
		lcore->ev_out_queue_sched_type = stage_config->ev_queue.sched_type_out;
		lcore->ev_out_queue = stage_config->ev_queue.out;
		break;
	case STAGE_TYPE_TX:
		lcore->ev_in_queue_needed = 1;
		lcore->ev_in_queue_sched_type = stage_config->ev_queue.sched_type_in;
		lcore->ev_in_queue = stage_config->ev_queue.in;
		strncpy(lcore->ev_in_queue_mp_name, stage_config->ev_queue.mp_name, RTE_MEMPOOL_NAMESIZE);
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

	return 0;
}

int
lcore_graph_populate(struct lcore_params *lcore, bool enable_graph_pcap)
{
	char const *node_name, *ev_node_name, *link_node_name;
	rte_node_t node_id, ev_node_id, link_node_id;
	struct rte_node_ethdev_rx_config rx_config;
	struct rte_node_ethdev_tx_config tx_config;
	char node_suffix[RTE_NODE_NAMESIZE];
	char pcap_filename[NAME_MAX];
	char const **node_patterns;
	uint16_t nb_node_patterns;
	uint16_t peer_link_id;
	int rc = -EINVAL;
	int i;

	if (!lcore->enabled)
		return 0;

	/* Reset graph config */
	memset(&lcore->graph_config, 0, sizeof(lcore->graph_config));
	nb_node_patterns = 0;
	node_patterns = rte_zmalloc(
		NULL,
		GRAPH_MAX_PATTERNS * sizeof(*node_patterns),
		0);

	rc = rte_event_port_setup(lcore->ev_id,
					lcore->ev_port_id,
					&lcore->ev_port_config);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	if (lcore->ev_in_queue_needed) {
		rc = rte_event_port_link(lcore->ev_id,
					lcore->ev_port_id,
					&lcore->ev_in_queue,
					NULL,
					1);
		if (rc < 0) {
			rc = -rte_errno;
			goto err;
		}

		snprintf(node_suffix, sizeof(node_suffix), 
			"%u-%u", lcore->ev_port_id, lcore->ev_in_queue);
		ev_node_id = eventdev_rx_node_clone(node_suffix);
		if (ev_node_id == RTE_NODE_ID_INVALID) {
			RTE_LOG(INFO, USER1, "Eventdev rx node (%s) create failed\n", node_suffix);
			rc = -ENOMEM;
			goto err;
		}

		ev_node_name = rte_node_id_to_name(ev_node_id);
		if (ev_node_name == NULL) {
			RTE_LOG(INFO, USER1, "Eventdev rx node (%s) get name failed\n", node_suffix);
			rc = -ENOENT;
			goto err;
		}

		rc = eventdev_rx_node_data_add(ev_node_id,
						lcore->ev_id,
						lcore->ev_port_id,
						lcore->ev_in_queue_mp_name);
		if (rc < 0) {
			RTE_LOG(INFO, USER1, "Eventdev rx node (%s) data add failed\n", ev_node_name);
			goto err;
		}
		node_patterns[nb_node_patterns++] = strdup(ev_node_name);

		if (lcore->ev_out_queue_needed) {
			rc = eventdev_dispatcher_set_mempool(lcore->ev_in_queue_mp_name);
			if (rc < 0) {
				RTE_LOG(INFO, USER1, "Eventdev dispatcher node set mempool (%s) failed\n", 
					lcore->ev_in_queue_mp_name);
				goto err;
			}

			node_patterns[nb_node_patterns++] = strdup("vs_eventdev_dispatcher");
		} else {
			snprintf(node_suffix, sizeof(node_suffix), "%u", lcore->ev_port_id);
			node_id = forward_node_clone(node_suffix);
			if (node_id == RTE_NODE_ID_INVALID) {
				RTE_LOG(INFO, USER1, "Forward node (%s) create failed\n", node_suffix);
				rc = -ENOMEM;
				goto err;
			}

			node_name = rte_node_id_to_name(node_id);
			if (node_name == NULL) {
				RTE_LOG(INFO, USER1, "Forward node (%s) get name failed\n", node_suffix);
				rc = -ENOENT;
				goto err;
			}

			rc = eventdev_rx_node_data_set_next(ev_node_id, node_name);
			if (rc < 0) {
				RTE_LOG(INFO, USER1, "Eventdev rx node (%s) set next (%s) failed\n", 
					ev_node_name, node_name);
				goto err;
			}

			node_patterns[nb_node_patterns++] = strdup(node_name);
			for (i = 0; i < lcore->nb_link_out_queues; i++) {
				tx_config.link_id = lcore->link_out_queues[i].link_id;
				tx_config.queue_id = lcore->link_out_queues[i].queue_id;
				link_node_id = rte_node_ethdev_tx_config(&tx_config);
				if (link_node_id == RTE_NODE_ID_INVALID) {
					RTE_LOG(INFO, USER1, "Ethdev tx node (%u:%u) create failed\n",
						tx_config.link_id, tx_config.queue_id);
					rc = -ENOMEM;
					goto err;
				}

				link_node_name = rte_node_id_to_name(link_node_id);
				if (link_node_name == NULL) {
					RTE_LOG(INFO, USER1, "Ethdev tx node (%u:%u) get name failed\n",
						tx_config.link_id, tx_config.queue_id);
					rc = -ENOENT;
					goto err;
				}

				node_patterns[nb_node_patterns++] = strdup(link_node_name);
				rc = link_get_peer(tx_config.link_id, &peer_link_id);
				if (rc < 0) {
					RTE_LOG(INFO, USER1, "link_get_peer (%u) failed\n", tx_config.link_id);
					continue;
				}

				rc = forward_node_data_add(
					node_id,
					peer_link_id,
					link_node_name);
				if (rc < 0) {
					RTE_LOG(INFO, USER1, "Forward node (%s) add (%s) failed\n",
						node_suffix, link_node_name);
					goto err;

				}
			}
		}
	}

	if (lcore->ev_out_queue_needed) {
		snprintf(node_suffix, sizeof(node_suffix), 
			"%u-%u", lcore->ev_port_id, lcore->ev_out_queue);
		ev_node_id = eventdev_tx_node_clone(node_suffix);
		if (ev_node_id == RTE_NODE_ID_INVALID) {
			RTE_LOG(INFO, USER1, "Eventdev tx node (%s) create failed\n", node_suffix);
			rc = -ENOMEM;
			goto err;
		}

		ev_node_name = rte_node_id_to_name(ev_node_id);
		if (ev_node_name == NULL) {
			RTE_LOG(INFO, USER1, "Eventdev tx node (%s) get name failed\n", node_suffix);
			rc = -ENOENT;
			goto err;
		}

		if (lcore->ev_in_queue_needed) {
			rc = eventdev_tx_node_data_add(ev_node_id,
						lcore->ev_id,
						lcore->ev_port_id,
						RTE_EVENT_OP_FORWARD,
						lcore->ev_out_queue_sched_type,
						lcore->ev_out_queue,
						RTE_EVENT_TYPE_CPU,
						0,
						RTE_EVENT_DEV_PRIORITY_NORMAL);
			if (rc < 0) {
				RTE_LOG(INFO, USER1, "Eventdev tx node (%s) data add failed\n",
					ev_node_name);
				goto err;
			}

			rc = eventdev_dispatcher_add_next(ev_node_name, lcore->core_id);
			if (rc < 0) {
				RTE_LOG(INFO, USER1, "Eventdev dispatcher node add next (%s) failed\n",
					ev_node_name);
				goto err;
			}

			node_patterns[nb_node_patterns++] = strdup(ev_node_name);
		} else {
			rc = eventdev_tx_node_data_add(ev_node_id,
						lcore->ev_id,
						lcore->ev_port_id,
						RTE_EVENT_OP_NEW,
						lcore->ev_out_queue_sched_type,
						lcore->ev_out_queue,
						RTE_EVENT_TYPE_ETHDEV,
						0,
						RTE_EVENT_DEV_PRIORITY_NORMAL);
			if (rc < 0) {
				RTE_LOG(INFO, USER1, "Eventdev tx node (%s) data add failed\n",
					ev_node_name);
				goto err;
			}

			node_patterns[nb_node_patterns++] = strdup(ev_node_name);
			for (i = 0; i < lcore->nb_link_in_queues; i++) {
				rx_config.link_id = lcore->link_in_queues[i].link_id;
				rx_config.queue_id = lcore->link_in_queues[i].queue_id;
				strncpy(rx_config.next_node, ev_node_name, sizeof(rx_config.next_node));
				link_node_id = rte_node_ethdev_rx_config(&rx_config);
				if (link_node_id == RTE_NODE_ID_INVALID) {
					RTE_LOG(INFO, USER1, "Ethdev rx node (%u:%u) create failed\n",
						rx_config.link_id, rx_config.queue_id);
					rc = -ENOMEM;
					goto err;
				}

				link_node_name = rte_node_id_to_name(link_node_id);
				if (link_node_name == NULL) {
					RTE_LOG(INFO, USER1, "Ethdev rx node (%u:%u) get name failed\n",
						rx_config.link_id, rx_config.queue_id);
					rc = -ENOENT;
					goto err;
				}

				node_patterns[nb_node_patterns++] = strdup(link_node_name);
			}
		}
	}

	node_name = strtok(lcore->nodes, ",");
	while (node_name != NULL) {
		node_patterns[nb_node_patterns++] = strdup(node_name);
		node_name = strtok(NULL, ",");
	}

	RTE_LOG(DEBUG, USER1, "Core (%u) create graph with patterns:\n", lcore->core_id);
	for (i = 0; i < nb_node_patterns; i++) {
		RTE_LOG(DEBUG, USER1, "\t%s\n", node_patterns[i]);
	}
	snprintf(lcore->graph_name, sizeof(lcore->graph_name),
		"worker_%u", lcore->core_id);
	lcore->graph_config.node_patterns = node_patterns;
	lcore->graph_config.nb_node_patterns = nb_node_patterns;
	if (enable_graph_pcap) {
		lcore->graph_config.pcap_enable = 1;
		lcore->graph_config.num_pkt_to_capture = 1000;
		snprintf(pcap_filename, sizeof(pcap_filename),
			"/tmp/worker_%u.pcap", lcore->core_id);
		lcore->graph_config.pcap_filename = strdup(pcap_filename);
	} else {
		lcore->graph_config.pcap_enable = 0;
	}

	return 0;

err:
	return rc;
}

int
lcore_graph_worker(void *arg)
{
	struct lcore_params *lcore = (struct lcore_params*) arg;

	RTE_LOG(INFO, USER1, "Lcore %u (%s) started\n", lcore->core_id, stage_type_str[lcore->type]);

	lcore->graph_id = rte_graph_create(lcore->graph_name, &lcore->graph_config);
	if (lcore->graph_id == RTE_GRAPH_ID_INVALID)
		rte_exit(EXIT_FAILURE,
				"rte_graph_create(): graph_id invalid"
				" for lcore %u\n", lcore->core_id);

	lcore->graph = rte_graph_lookup(lcore->graph_name);
	if (!lcore->graph)
		rte_exit(EXIT_FAILURE,
				"rte_graph_lookup(): graph %s not found\n",
				lcore->graph_name);

	while(1) {
		rte_graph_walk(lcore->graph);
	}

	RTE_LOG(INFO, USER1, "Lcore %u (%s) stopped\n", lcore->core_id, stage_type_str[lcore->type]);

	return 0;
}

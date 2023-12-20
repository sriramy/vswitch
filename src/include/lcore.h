/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_LCORE_H_
#define __VSWITCH_SRC_API_LCORE_H_

#include <rte_eventdev.h>
#include <rte_graph.h>

#include "stage.h"

#define EV_QUEUE_ID_INVALID	(0xFF)
#define GRAPH_MAX_PATTERNS	(16)

struct lcore_params {
	uint16_t core_id;
	uint16_t enabled;
	uint8_t ev_id;
	uint8_t ev_port_id;
	uint8_t type;
	uint8_t ev_in_queue_needed;
	uint8_t ev_in_queue_sched_type;
	uint8_t ev_in_queue;
	char ev_in_queue_mp_name[RTE_MEMPOOL_NAMESIZE];
	uint8_t ev_out_queue_needed;
	uint8_t ev_out_queue_sched_type;
	uint8_t ev_out_queue;
	struct rte_event_port_conf ev_port_config;
	uint8_t nb_link_in_queues;
	uint8_t nb_link_out_queues;
	struct {
		uint16_t link_id;
		uint8_t queue_id;
	} link_in_queues[STAGE_MAX_LINK_QUEUES];
	struct {
		uint16_t link_id;
		uint8_t queue_id;
	} link_out_queues[STAGE_MAX_LINK_QUEUES];

	char nodes[STAGE_GRAPH_NODES_MAX_LEN];
	struct rte_graph_param graph_config;
	struct rte_graph *graph;
	char graph_name[RTE_GRAPH_NAMESIZE];
	rte_graph_t graph_id;
} __rte_cache_aligned;

void lcore_init(uint16_t core_id, uint8_t ev_id, struct lcore_params *lcore);
int lcore_config_populate(struct stage_config *stage_config, uint8_t ev_port_id, struct lcore_params *lcore);
int lcore_graph_populate(struct lcore_params *lcore, bool enable_graph_pcap);
int lcore_graph_worker(void *arg);


#endif /* __VSWITCH_SRC_API_LCORE_H_ */
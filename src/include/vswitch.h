/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_VSWITCH_H_
#define __VSWITCH_SRC_API_VSWITCH_H_

#include <rte_eventdev.h>
#include <rte_graph.h>

#define EV_QUEUE_ID_INVALID	(0xFF)

struct link_queues {
	uint16_t link_id;
	uint8_t queue_id;
};

struct lcore_params {
	uint16_t core_id;
	uint16_t enabled;
	uint16_t ev_id;
	uint16_t ev_port_id;
	uint8_t type;
	uint8_t ev_in_queue_needed;
	uint8_t ev_in_queue;
	uint8_t ev_out_queue_needed;
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

	struct rte_graph *graph;
	char graph_name[RTE_GRAPH_NAMESIZE];
	rte_graph_t graph_id;

} __rte_cache_aligned;

struct vswitch_config {
	int nb_ports;
	int nb_queues;
	int ev_id;
	int ev_service_id;
	struct rte_event_dev_info ev_info;
	struct lcore_params lcores[RTE_MAX_LCORE];
};

int vswitch_init();
int vswitch_quit();
struct vswitch_config* vswitch_config_get();

int vswitch_start();

#endif /* __VSWITCH_SRC_API_VSWITCH_H_ */
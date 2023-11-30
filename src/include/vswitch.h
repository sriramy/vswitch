/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_VSWITCH_H_
#define __VSWITCH_SRC_API_VSWITCH_H_

#include <rte_eventdev.h>

#define EV_QUEUE_ID_INVALID	(0xFF)

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
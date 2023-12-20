/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_VSWITCH_H_
#define __VSWITCH_SRC_API_VSWITCH_H_

#include <rte_eventdev.h>
#include <rte_graph.h>

#include "lcore.h"
#include "options.h"

struct vswitch_config {
	struct params params;
	int nb_ports;
	int nb_queues;
	int ev_id;
	int ev_service_id;
	struct rte_event_dev_info ev_info;
	struct lcore_params lcores[RTE_MAX_LCORE];
};

int vswitch_init(struct params *p);
int vswitch_quit();
struct vswitch_config* vswitch_config_get();

int vswitch_start();
int vswitch_dump_stats(char const *file);

#endif /* __VSWITCH_SRC_API_VSWITCH_H_ */
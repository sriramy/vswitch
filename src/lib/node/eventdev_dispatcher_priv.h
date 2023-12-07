/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_EVENTDEV_DISPATCHER_PRIV_H__
#define __SRC_LIB_EVENTDEV_DISPATCHER_PRIV_H__

#include <rte_common.h>
#include <rte_graph.h>

enum eventdev_dispatcher_next_nodes {
	EVENTDEV_DISPATCHER_NEXT_PKT_DROP,
	EVENTDEV_DISPATCHER_NEXT_MAX,
};

struct eventdev_dispatcher_node_data {
	struct {
		rte_edge_t id;
		uint8_t enabled;
	} next_ethdev[RTE_MAX_ETHPORTS][RTE_MAX_QUEUES_PER_PORT];
	struct {
		rte_edge_t id;
		uint8_t enabled;
	} next_eventdev[RTE_MAX_LCORE];
};

struct eventdev_dispatcher_node_ctx {
	uint16_t port_id;
	struct eventdev_dispatcher_node_data *node_data;
};

#endif /* __SRC_LIB_EVENTDEV_DISPATCHER_PRIV_H__ */

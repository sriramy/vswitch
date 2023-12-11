/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_NODE_EVENTDEV_DISPATCHER_PRIV_H__
#define __SRC_LIB_NODE_EVENTDEV_DISPATCHER_PRIV_H__

#include <rte_common.h>
#include <rte_graph.h>
#include <rte_mempool.h>

enum eventdev_dispatcher_next_nodes {
	EVENTDEV_DISPATCHER_NEXT_PKT_DROP,
	EVENTDEV_DISPATCHER_NEXT_MAX,
};

struct eventdev_dispatcher_node_data {
	struct {
		rte_edge_t id;
		uint8_t enabled;
	} next[RTE_MAX_LCORE];
	struct rte_mempool *mp;
};

struct eventdev_dispatcher_node_ctx {
	uint16_t port_id;
	struct eventdev_dispatcher_node_data *data;
};

#endif /* __SRC_LIB_NODE_EVENTDEV_DISPATCHER_PRIV_H__ */

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

struct eventdev_dispatcher_node_ctx {
	struct {
		rte_edge_t id;
		uint8_t enabled;
	} next_index[RTE_MAX_ETHPORTS][RTE_MAX_QUEUES_PER_PORT];
};

#endif /* __SRC_LIB_EVENTDEV_DISPATCHER_PRIV_H__ */

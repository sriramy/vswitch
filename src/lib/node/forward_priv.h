/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_NODE_FORWARD_PRIV_H__
#define __SRC_LIB_NODE_FORWARD_PRIV_H__

#include <rte_common.h>
#include <rte_graph.h>

enum forward_next_nodes {
	FORWARD_NEXT_PKT_DROP = 0,
	FORWARD_NEXT_MAX,
};

struct forward_node_ctx {
	struct {
		rte_edge_t id;
		uint8_t enabled;
	} next_nodes[RTE_MAX_ETHPORTS];
};

struct forward_node_item {
        struct forward_node_item *next;
        struct forward_node_item *prev;
        struct forward_node_ctx ctx;
        rte_node_t node_id;
};

struct forward_node_list {
        struct forward_node_item *head;
};

#endif /* __SRC_LIB_NODE_FORWARD_PRIV_H__ */

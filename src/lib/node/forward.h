/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_NODE_FORWARD_H__
#define __SRC_LIB_NODE_FORWARD_H__

#include <rte_graph.h>

rte_node_t forward_node_clone(char const *name);

int forward_node_data_add(rte_node_t node_id, uint16_t link_id, const char *next_node);
int forward_node_data_rem(rte_node_t node_id);

#endif /* __SRC_LIB_NODE_FORWARD_H__ */

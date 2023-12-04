/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_EVENTDEV_RX_H__
#define __SRC_LIB_EVENTDEV_RX_H__

#include <rte_graph.h>

rte_node_t eventdev_rx_node_clone(const char *name);

int eventdev_rx_node_data_add(rte_node_t node_id, struct eventdev_rx_node_ctx ctx);
int eventdev_rx_node_data_rem(rte_node_t node_id);

int eventdev_rx_node_data_add_next_node(rte_node_t id, const char *edge_name);

#endif /* __SRC_LIB_EVENTDEV_RX_H__ */

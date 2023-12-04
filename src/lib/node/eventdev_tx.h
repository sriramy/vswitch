/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_EVENTDEV_TX_H__
#define __SRC_LIB_EVENTDEV_TX_H__

#include <rte_graph.h>

rte_node_t eventdev_tx_node_clone(const char *name);

int eventdev_tx_node_data_add(rte_node_t node_id, struct eventdev_tx_node_ctx ctx);
int eventdev_tx_node_data_rem(rte_node_t node_id);

#endif /* __SRC_LIB_EVENTDEV_TX_H__ */

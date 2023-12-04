/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_EVENTDEV_RX_PRIV_H__
#define __SRC_LIB_EVENTDEV_RX_PRIV_H__

#include <rte_common.h>
#include <rte_graph.h>

struct eventdev_rx_node_ctx {
        uint16_t ev_id;
	uint16_t ev_port_id;
        rte_node_t next_node;
};

struct eventdev_rx_node_item {
        struct eventdev_rx_node_item *next;
        struct eventdev_rx_node_item *prev;
        struct eventdev_rx_node_ctx ctx;
        rte_node_t node_id;
};

struct eventdev_rx_node_list {
        struct eventdev_rx_node_item *head;
};

int eventdev_rx_node_data_add(rte_node_t node_id, struct eventdev_rx_node_ctx ctx);
int eventdev_rx_node_data_rem(rte_node_t node_id);
struct eventdev_rx_node_item *eventdev_rx_node_data_get(rte_node_t node_id);

rte_node_t eventdev_rx_node_clone(const char *name);

#endif /* __SRC_LIB_EVENTDEV_RX_PRIV_H__ */
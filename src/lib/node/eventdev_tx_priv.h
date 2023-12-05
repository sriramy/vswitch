/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_EVENTDEV_TX_PRIV_H__
#define __SRC_LIB_EVENTDEV_TX_PRIV_H__

#include <rte_common.h>
#include <rte_graph.h>

enum eventdev_tx_next_nodes {
	EVENTDEV_TX_NEXT_PKT_DROP,
	EVENTDEV_TX_NEXT_MAX,
};

struct eventdev_tx_node_ctx {
        uint16_t ev_id;
	uint16_t ev_port_id;
        uint8_t op;
        uint8_t sched_type;
        uint8_t queue_id;
        uint8_t event_type;
        uint8_t sub_event_type;
        uint8_t priority;
};

struct eventdev_tx_node_item {
        struct eventdev_tx_node_item *next;
        struct eventdev_tx_node_item *prev;
        struct eventdev_tx_node_ctx ctx;
        rte_node_t node_id;
};

struct eventdev_tx_node_list {
        struct eventdev_tx_node_item *head;
};

#endif /* __SRC_LIB_EVENTDEV_TX_PRIV_H__ */

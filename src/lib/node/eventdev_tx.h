/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_NODE_EVENTDEV_TX_H__
#define __SRC_LIB_NODE_EVENTDEV_TX_H__

#include <rte_graph.h>

rte_node_t eventdev_tx_node_clone(char const *name);

int eventdev_tx_node_data_add(rte_node_t node_id,
			      uint16_t ev_id,
			      uint16_t ev_port_id,
			      uint8_t op,
			      uint8_t sched_type,
			      uint8_t queue_id,
			      uint8_t event_type,
			      uint8_t sub_event_type,
			      uint8_t priority);
int eventdev_tx_node_data_rem(rte_node_t node_id);

#endif /* __SRC_LIB_NODE_EVENTDEV_TX_H__ */

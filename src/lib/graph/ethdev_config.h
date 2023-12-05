/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_LIB_GRAPH_ETHDEV_CONFIG_H__
#define __VSWITCH_SRC_LIB_GRAPH_ETHDEV_CONFIG_H__

struct rte_node_ethdev_rx_config {
	uint16_t link_id;
	uint8_t queue_id;
	char next_node[RTE_NODE_NAMESIZE];
};

int rte_node_ethdev_rx_config(struct rte_node_ethdev_rx_config *config);

#endif /* __VSWITCH_SRC_LIB_GRAPH_ETHDEV_CONFIG_H__ */
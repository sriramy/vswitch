/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_graph.h>

#include <rte_node_eth_api.h>
#include "ethdev_rx_priv.h"

#include "ethdev_config.h"
#include "node/eventdev_rx.h"

int
rte_node_ethdev_rx_config(struct rte_node_ethdev_rx_config *config)
{
	struct ethdev_rx_node_main *rx_node_data;
	struct rte_node_register *rx_node;
	char name[RTE_NODE_NAMESIZE];
	ethdev_rx_node_elem_t *elem;

	rx_node_data = ethdev_rx_get_node_data_get();
	rx_node = ethdev_rx_node_get();
	snprintf(name, sizeof(name), "%u-%u", config->link_id, config->queue_id);

	id = rte_node_clone(rx_node->id, name);
	if (id == RTE_NODE_ID_INVALID)
		return -EIO;

	rte_node_edge_update(id, RTE_EDGE_ID_INVALID, &config->next_node, 1);

	elem = malloc(sizeof(ethdev_rx_node_elem_t));
	if (elem == NULL)
		return -ENOMEM;

	memset(elem, 0, sizeof(ethdev_rx_node_elem_t));
	elem->ctx.port_id = config->link_id;
	elem->ctx.queue_id = config->queue_id;
	elem->ctx.cls_next = rte_node_edge_count(id) - 1;
	elem->nid = id;
	elem->next = rx_node_data->head;
	rx_node_data->head = elem;
}


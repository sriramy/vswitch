/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <errno.h>
#include <string.h>

#include <rte_malloc.h>
#include <rte_eventdev.h>
#include <rte_graph.h>
#include <rte_graph_worker.h>
#include <rte_mbuf.h>

#include "eventdev_rx_priv.h"
#include "eventdev_rx.h"

static struct eventdev_rx_node_list node_list = {
	.head = NULL,
};

static struct eventdev_rx_node_item* eventdev_rx_node_data_get(rte_node_t node_id);

int
eventdev_rx_node_data_add_next_node(rte_node_t id, char const *edge_name) {
	struct eventdev_rx_node_item *item;
	char **next_nodes;
	int i, rc = -EIO;
	uint32_t count;

	if (edge_name == NULL)
		return -EINVAL;

	count = rte_node_edge_get(id, NULL);
	if (count == RTE_EDGE_ID_INVALID)
		return -EINVAL;

	next_nodes = rte_malloc(NULL, count, 0);
	if (!next_nodes)
		return -ENOMEM;

	count = rte_node_edge_get(id, next_nodes);
	for (i = 0; next_nodes[i]; i++) {
		if (strcmp(edge_name, next_nodes[i]) == 0) {
			item = eventdev_rx_node_data_get(id);
			if (item) {
				item->ctx.next_node = i;
				rc = 0;
				break;
			}
		}
		i++;
	}

	rte_free(next_nodes);
	return rc;
}

int
eventdev_rx_node_data_add(rte_node_t node_id, uint8_t ev_id, uint8_t ev_port_id, char const *event_mempool)
{
	struct eventdev_rx_node_item* item;

	item = eventdev_rx_node_data_get(node_id);
	if (item)
		return -EINVAL;

	item = rte_zmalloc(NULL, sizeof(struct eventdev_rx_node_item), 0);
	if (!item)
		return -ENOMEM;

	item->node_id = node_id;
	item->ctx.ev_id = ev_id;
	item->ctx.ev_port_id = ev_port_id;
	item->ctx.mp = rte_mempool_lookup(event_mempool);
	item->ctx.next_node = EVENTDEV_RX_NEXT_DISPATCHER;
	item->prev = NULL;
	item->next = node_list.head;
	node_list.head = item;

	return 0;
}

int
eventdev_rx_node_data_rem(rte_node_t node_id)
{
	struct eventdev_rx_node_item* item;

	item = eventdev_rx_node_data_get(node_id);
	if (!item)
		return -ENOENT;

	if (item->next)
		item->next->prev = item->prev;

	if (item->prev)
		item->prev->next = item->next;

	if (item == node_list.head)
		node_list.head = item->next;

	rte_free(item);
	return 0;
}

static struct eventdev_rx_node_item*
eventdev_rx_node_data_get(rte_node_t node_id)
{
	struct eventdev_rx_node_item *item = node_list.head;

	for (; item; item = item->next) {
		if (item->node_id == node_id)
			return item;
	}

	return NULL;
}

static __rte_always_inline uint16_t
eventdev_rx_node_process(struct rte_graph *graph,
			 struct rte_node *node,
			 __rte_unused void **objs,
			 __rte_unused uint16_t cnt)
{
	struct eventdev_rx_node_ctx *ctx = (struct eventdev_rx_node_ctx *)node->ctx;
	struct rte_event events[RTE_GRAPH_BURST_SIZE];
	uint16_t n_events = 0;
	int i, timeout = 0;

	if (likely(rte_mempool_get_bulk(ctx->mp, node->objs, RTE_GRAPH_BURST_SIZE)) == 0) {
		n_events = rte_event_dequeue_burst(ctx->ev_id,
						   ctx->ev_port_id,
						   events,
						   RTE_GRAPH_BURST_SIZE,
						   timeout);
		if (n_events) {
			for (i = 0; i < n_events; i++) {
				node->objs[i] = &events[i];
			}
			node->idx = n_events;
			rte_node_next_stream_move(graph, node, ctx->next_node);
		}
	}

	return n_events;
}

static int
eventdev_rx_node_init(__rte_unused const struct rte_graph *graph, struct rte_node *node)
{
	struct eventdev_rx_node_ctx *ctx = (struct eventdev_rx_node_ctx *)node->ctx;
	struct eventdev_rx_node_item *item = eventdev_rx_node_data_get(node->id);

	if (item)
		memcpy(ctx, &item->ctx, sizeof(item->ctx));

	RTE_VERIFY(item != NULL);

	return 0;
}

static struct rte_node_register eventdev_rx_node = {
	.process = eventdev_rx_node_process,
	.flags = RTE_NODE_SOURCE_F,
	.name = "vs_eventdev_rx",

	.init = eventdev_rx_node_init,

	.nb_edges = EVENTDEV_RX_NEXT_MAX,
	.next_nodes = {
		[EVENTDEV_RX_NEXT_DISPATCHER] = "vs_eventdev_dispatcher",
		[EVENTDEV_RX_NEXT_PKT_DROP] = "pkt_drop",
	},
};

rte_node_t
eventdev_rx_node_clone(char const *name)
{
	return rte_node_clone(eventdev_rx_node.id, name);
}

RTE_NODE_REGISTER(eventdev_rx_node);

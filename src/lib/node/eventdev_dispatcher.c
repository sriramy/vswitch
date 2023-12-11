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
#include <rte_mempool.h>

#include "eventdev_dispatcher_priv.h"
#include "eventdev_dispatcher.h"

static struct eventdev_dispatcher_node_data node_main;

static __rte_always_inline uint16_t
eventdev_dispatcher_node_process(struct rte_graph *graph,
			 struct rte_node *node,
			 void **events,
			 uint16_t count)
{
	struct eventdev_dispatcher_node_ctx *ctx = (struct eventdev_dispatcher_node_ctx *)node->ctx;
	uint8_t next_index = EVENTDEV_DISPATCHER_NEXT_PKT_DROP;
	struct rte_event *event;
	int i;

	for (i = 0; i < count; i++) {
		event = (struct rte_event *)events[i];
		if (ctx->data->next[ctx->port_id].enabled)
			next_index = ctx->data->next[ctx->port_id].id;
		rte_node_enqueue(graph,
				 node,
				 next_index,
				 (void**) &event->mbuf,
				 1);
	}

	rte_mempool_put_bulk(ctx->data->mp, (void**)events, count);
	return count;
}

int
eventdev_dispatcher_set_mempool(char const* mp_name)
{
	rte_node_t id;

	id = rte_node_from_name("vs_eventdev_dispatcher");
	if (id == RTE_NODE_ID_INVALID)
		return -EIO;

	node_main.mp = rte_mempool_lookup(mp_name);
	if (!node_main.mp)
		return -ENOENT;

	return 0;
}

int
eventdev_dispatcher_add_next(char const* next_node, uint16_t port_id)
{
	rte_node_t id;

	id = rte_node_from_name("vs_eventdev_dispatcher");
	if (id == RTE_NODE_ID_INVALID)
		return -EIO;

	rte_node_edge_update(id, RTE_EDGE_ID_INVALID, &next_node, 1);

	node_main.next[port_id].enabled = 1;
	node_main.next[port_id].id = rte_node_edge_count(id) - 1;

	return 0;
}

static int
eventdev_dispatcher_node_init(__rte_unused const struct rte_graph *graph, struct rte_node *node)
{
	struct eventdev_dispatcher_node_ctx *ctx = (struct eventdev_dispatcher_node_ctx *)node->ctx;

	ctx->port_id = rte_lcore_id();
	ctx->data = &node_main;

	return 0;
}

static struct rte_node_register eventdev_dispatcher_node = {
	.process = eventdev_dispatcher_node_process,
	.name = "vs_eventdev_dispatcher",

	.init = eventdev_dispatcher_node_init,

	.nb_edges = EVENTDEV_DISPATCHER_NEXT_MAX,
	.next_nodes = {
		[EVENTDEV_DISPATCHER_NEXT_PKT_DROP] = "pkt_drop",
	},
};

RTE_NODE_REGISTER(eventdev_dispatcher_node);

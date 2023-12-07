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

#include "eventdev_dispatcher_priv.h"
#include "eventdev_dispatcher.h"

#define DEFAULT_PKT_BURST (32)
#define DEFAULT_EVENT_BURST (32)

struct eventdev_dispatcher_node_ctx *node_ctx = NULL;

static __rte_always_inline uint16_t
eventdev_dispatcher_node_process(struct rte_graph *graph,
			 struct rte_node *node,
			 void **events,
			 uint16_t count)
{
	struct eventdev_dispatcher_node_ctx *ctx = (struct eventdev_dispatcher_node_ctx *)node->ctx;
	uint8_t next_index = EVENTDEV_DISPATCHER_NEXT_PKT_DROP;
	struct rte_event *event;
	uint16_t n_pkts = 0;
	int i;

	for (i = 0; i < count; i++) {
		event = (struct rte_event *)events[i];
		if (ctx->next_index[event->mbuf->port][0].enabled) {
			next_index = ctx->next_index[event->mbuf->port][0].id;
		}
		rte_node_enqueue(graph,
				 node,
				 next_index,
				 (void**) &event->mbuf,
				 count - n_pkts);
	}

	return count;
}

int
eventdev_dispatcher_set_next(const char* next_node, uint16_t port_id, uint16_t queue_id)
{
	rte_node_t id;

	if (!node_ctx)
		return -EINVAL;

	id = rte_node_from_name("vs_eventdev_dispatcher");
	if (id == RTE_NODE_ID_INVALID)
		return -EIO;

	rte_node_edge_update(id, RTE_EDGE_ID_INVALID, &next_node, 1);

	node_ctx->next_index[port_id][queue_id].enabled = 1;
	node_ctx->next_index[port_id][queue_id].id = rte_node_edge_count(id) - 1; 

	return 0;
}

static int
eventdev_dispatcher_node_init(__rte_unused const struct rte_graph *graph, struct rte_node *node)
{
	struct eventdev_dispatcher_node_ctx *ctx = (struct eventdev_dispatcher_node_ctx *)node->ctx;

	memset(ctx->next_index, 0, sizeof(ctx->next_index));
	node_ctx = ctx;

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

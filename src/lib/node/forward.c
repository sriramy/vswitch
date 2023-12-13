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

#include "forward_priv.h"
#include "forward.h"

static struct forward_node_list node_list = {
	.head = NULL,
};

static struct forward_node_item* forward_node_data_get(rte_node_t node_id);

int
forward_node_data_add(rte_node_t node_id, uint16_t link_id, const char *next_node)
{
	struct forward_node_item* item;

	rte_node_edge_update(node_id, RTE_EDGE_ID_INVALID, &next_node, 1);

	item = forward_node_data_get(node_id);
	if (!item) {
		item = rte_zmalloc(NULL, sizeof(struct forward_node_item), 0);
		if (!item)
			return -ENOMEM;

		memset(&item->ctx, 0, sizeof(item->ctx));
		item->node_id = node_id;
		item->prev = NULL;
		item->next = node_list.head;
		node_list.head = item;
	}

	item->ctx.next_nodes[link_id].enabled = 1;
	item->ctx.next_nodes[link_id].id = rte_node_edge_count(node_id) - 1;

	return 0;
}

int
forward_node_data_rem(rte_node_t node_id)
{
	struct forward_node_item* item;

	item = forward_node_data_get(node_id);
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

static struct forward_node_item*
forward_node_data_get(rte_node_t node_id)
{
	struct forward_node_item *item = node_list.head;

	for (; item; item = item->next) {
		if (item->node_id == node_id)
			return item;
	}

	return NULL;
}

static __rte_always_inline uint16_t
forward_node_process(struct rte_graph *graph,
			 struct rte_node *node,
			 void **mbufs,
			 uint16_t count)
{
	struct forward_node_ctx *ctx = (struct forward_node_ctx *)node->ctx;
	uint8_t next_index = FORWARD_NEXT_PKT_DROP;
	struct rte_mbuf *mbuf;
	int i;

	for (i = 0; i < count; i++) {
		mbuf = (struct rte_mbuf*)mbufs[i];
		if (ctx->next_nodes[mbuf->port].enabled)
			next_index = ctx->next_nodes[mbuf->port].id;
		rte_node_enqueue(graph,
				 node,
				 next_index,
				 (void**) &mbuf[i],
				 1);
	}

	return count;
}

static int
forward_node_init(__rte_unused const struct rte_graph *graph, struct rte_node *node)
{
	struct forward_node_ctx *ctx = (struct forward_node_ctx *)node->ctx;
	struct forward_node_item *item = forward_node_data_get(node->id);

	if (item)
		memcpy(ctx, &item->ctx, sizeof(node->ctx));

	RTE_VERIFY(item != NULL);

	return 0;
}

static struct rte_node_register forward_node = {
	.process = forward_node_process,
	.name = "vs_forward",

	.init = forward_node_init,

	.nb_edges = FORWARD_NEXT_MAX,
	.next_nodes = {
		[FORWARD_NEXT_PKT_DROP] = "pkt_drop",
	},
};

rte_node_t
forward_node_clone(char const *name)
{
	return rte_node_clone(forward_node.id, name);
}

RTE_NODE_REGISTER(forward_node);

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

#include "eventdev_tx_priv.h"
#include "eventdev_tx.h"

#define DEFAULT_PKT_BURST (32)
#define DEFAULT_EVENT_BURST (32)

static struct eventdev_tx_node_list node_list = {
	.head = NULL,
};

static struct eventdev_tx_node_item* eventdev_tx_node_data_get(rte_node_t node_id);

int
eventdev_tx_node_data_add(rte_node_t node_id,
			  uint16_t ev_id,
			  uint16_t ev_port_id,
			  uint8_t op,
			  uint8_t sched_type,
			  uint8_t queue_id,
			  uint8_t event_type,
			  uint8_t sub_event_type,
			  uint8_t priority)
{
	struct eventdev_tx_node_item* item;

	item = eventdev_tx_node_data_get(node_id);
	if (item)
		return -EINVAL;

	item = rte_malloc(NULL, sizeof(struct eventdev_tx_node_item), 0);
	if (!item)
		return -ENOMEM;

	item->node_id = node_id;
	item->ctx.ev_id = ev_id;
	item->ctx.ev_port_id = ev_port_id;
	item->ctx.op = op;
	item->ctx.sched_type = sched_type;
	item->ctx.queue_id = queue_id;
	item->ctx.event_type = event_type;
	item->ctx.sub_event_type = sub_event_type;
	item->ctx.priority = priority;
	item->prev = NULL;
	item->next = node_list.head;
	node_list.head = item;

	return 0;
}

int
eventdev_tx_node_data_rem(rte_node_t node_id)
{
	struct eventdev_tx_node_item* item;

	item = eventdev_tx_node_data_get(node_id);
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

static struct eventdev_tx_node_item*
eventdev_tx_node_data_get(rte_node_t node_id)
{
	struct eventdev_tx_node_item *item = node_list.head;

	for (; item; item = item->next) {
		if (item->node_id == node_id)
			return item;
	}

	return NULL;
}

static __rte_always_inline uint16_t
eventdev_tx_node_process(struct rte_graph *graph,
			 struct rte_node *node,
			 void **mbufs,
			 uint16_t count)
{
	struct eventdev_tx_node_ctx *ctx = (struct eventdev_tx_node_ctx *)node->ctx;
	struct rte_event events[DEFAULT_EVENT_BURST];
	uint16_t n_pkts = 0;
	int i;

	for (i = 0; i < count; i++) {
		events[i].op = ctx->op;
		events[i].queue_id = ctx->queue_id;
		events[i].sched_type = ctx->sched_type;
		events[i].mbuf = mbufs[i];
		if (ctx->op == RTE_EVENT_OP_NEW) {
			events[i].flow_id = ((struct rte_mbuf*)mbufs[i])->hash.rss;
			events[i].event_type = ctx->event_type;
			events[i].sub_event_type = ctx->sub_event_type;
			events[i].priority = ctx->priority;
		}
	}

	n_pkts = rte_event_enqueue_burst(ctx->ev_id,
					 ctx->ev_port_id,
					 events,
					 count);

	if (n_pkts != count) {
		rte_node_enqueue(graph,
				 node,
				 EVENTDEV_TX_NEXT_PKT_DROP,
				 &mbufs[count],
				 count - n_pkts);
	}

	return count;
}

static int
eventdev_tx_node_init(__rte_unused const struct rte_graph *graph, struct rte_node *node)
{
	struct eventdev_tx_node_ctx *ctx = (struct eventdev_tx_node_ctx *)node->ctx;
	struct eventdev_tx_node_item *item = eventdev_tx_node_data_get(node->id);

	if (item)
		memcpy(ctx, &item->ctx, sizeof(item->ctx));

	RTE_VERIFY(item != NULL);

	return 0;
}

static struct rte_node_register eventdev_tx_node = {
	.process = eventdev_tx_node_process,
	.name = "vs_eventdev_tx",

	.init = eventdev_tx_node_init,

	.nb_edges = EVENTDEV_TX_NEXT_MAX,
	.next_nodes = {
		[EVENTDEV_TX_NEXT_PKT_DROP] = "pkt_drop",
	},
};

rte_node_t
eventdev_tx_node_clone(const char *name)
{
	return rte_node_clone(eventdev_tx_node.id, name);
}

RTE_NODE_REGISTER(eventdev_tx_node);

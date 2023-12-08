/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_MEMPOOL_H_
#define __VSWITCH_SRC_API_MEMPOOL_H_

#include <sys/queue.h>

#include <rte_ethdev.h>
#include <rte_mempool.h>

enum {
	MEMPOOL_TYPE_PKTMBUF = 0,
	MEMPOOL_TYPE_EVENT,
	MEMPOOL_TYPE_MAX
};

struct mempool_config {
	char name[RTE_MEMPOOL_NAMESIZE];
	uint8_t type;
	int nb_items;
	int item_sz;
	int cache_sz;
	int numa_node;
};

struct mempool {
	TAILQ_ENTRY(mempool) next;
	struct mempool_config config;
        struct rte_mempool *mp;
};
TAILQ_HEAD(mempool_head, mempool);

struct mempool *mempool_config_get(char const *name);
int mempool_config_add(struct mempool_config *config);
int mempool_config_rem(char const *name);

#endif /*__VSWITCH_SRC_API_MEMPOOL_H_ */
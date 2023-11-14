/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_MEMPOOL_H_
#define __VSWITCH_SRC_API_MEMPOOL_H_

#include <sys/queue.h>

#include <rte_ethdev.h>
#include <rte_mempool.h>

struct mempool_config {
	char name[RTE_MEMPOOL_NAMESIZE];
	int nb_mbufs;
	int mbuf_sz;
	int cache_sz;
	int numa_node;
};

struct mempool {
	TAILQ_ENTRY(mempool) next;
	struct mempool_config config;
        struct rte_mempool *mp;
};
TAILQ_HEAD(mempool_head, mempool);

#endif /*__VSWITCH_SRC_API_MEMPOOL_H_ */
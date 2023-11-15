/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_LINK_H_
#define __VSWITCH_SRC_API_LINK_H_

#include <rte_ethdev.h>
#include <rte_mempool.h>

#define ETHDEV_RXQ_RSS_MAX	16
#define ETHDEV_RX_DESC_DEFAULT 1024
#define ETHDEV_TX_DESC_DEFAULT 1024

struct link_rss_config {
	uint32_t queue_id[ETHDEV_RXQ_RSS_MAX];
	uint32_t n_queues;
};

struct link_config {
	char link_name[RTE_ETH_NAME_MAX_LEN];
	uint16_t link_id;
	int numa_node;

	struct {
		uint32_t nb_queues;
		uint32_t queue_sz;
		char mp_name[RTE_MEMPOOL_NAMESIZE];
		struct rte_mempool *mp;
		struct link_rss_config *rss;
	} rx;

	struct {
		uint32_t nb_queues;
		uint32_t queue_sz;
	} tx;

	int promiscuous;
	uint32_t mtu;
};

struct link {
	TAILQ_ENTRY(link) next;
	struct link_config config;
};
TAILQ_HEAD(link_head, link);

struct rte_eth_conf *link_config_default_get();

struct link *link_config_get(char const *name);
int link_config_add(struct link_config *l);
int link_config_rem(char const *name);

int link_config_set_promiscuous(const char *name, bool enable);
int link_config_set_mtu(const char *name, uint32_t mtu);

#endif /* __VSWITCH_SRC_API_LINK_H_ */
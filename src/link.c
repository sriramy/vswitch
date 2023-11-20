/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include <rte_malloc.h>

#include "link.h"

static struct link_head link_node = TAILQ_HEAD_INITIALIZER(link_node);

static struct rte_eth_conf link_conf_default = {
	.link_speeds = 0,
	.rxmode = {
		.mq_mode = RTE_ETH_MQ_RX_NONE,
		.mtu = 9000 - (RTE_ETHER_HDR_LEN + RTE_ETHER_CRC_LEN), /* Jumbo frame MTU */
	},
	.rx_adv_conf = {
		.rss_conf = {
			.rss_key = NULL,
			.rss_key_len = 40,
			.rss_hf = 0,
		},
	},
	.txmode = {
		.mq_mode = RTE_ETH_MQ_TX_NONE,
	},
	.lpbk_mode = 0,
};

struct rte_eth_conf *
link_config_default_get()
{
        return &link_conf_default;
}

struct link*
link_config_get(char const *name)
{
	struct link *l;

	TAILQ_FOREACH(l, &link_node, next) {
		if (strcmp(l->config.link_name, name) == 0)
			return l;
	}
	return NULL;
}

int
link_config_add(struct link_config *config)
{
	struct rte_eth_conf link_conf;
	int rc = -EINVAL;
	struct link* l;
	uint32_t i;

	rc = rte_eth_dev_get_port_by_name(config->link_name, &config->link_id);
	if (rc < 0) {
                rc = -rte_errno;
                goto err;
        }

	l = link_config_get(config->link_name);
	if (!l) {
		l = rte_malloc(NULL, sizeof(struct link), 0);
		if (!l) {
                        rc = -rte_errno;
                        goto err;
                }
	} else {
		return -EEXIST;
        }

	memcpy(&link_conf, link_config_default_get(), sizeof(struct rte_eth_conf));
	config->peer.link_name[0] = '\0';
	config->peer.link_id = LINK_ID_MAX;
	config->mtu = link_conf.rxmode.mtu;
	config->rx.mp = rte_mempool_lookup(config->rx.mp_name);
	if (!config->rx.mp) {
                rc = -rte_errno;
		goto err;
	}

	config->numa_node = rte_eth_dev_socket_id(config->link_id);
	if (config->numa_node == SOCKET_ID_ANY)
		config->numa_node = 0;

	memcpy(&l->config, config, sizeof(*config));
	rc = rte_eth_dev_configure(
		l->config.link_id,
		l->config.rx.nb_queues,
		l->config.tx.nb_queues,
		&link_conf);
	if (rc < 0) {
                rc = -rte_errno;
		goto err;
	}

	/* Port RX */
	for (i = 0; i < l->config.rx.nb_queues; i++) {
		rc = rte_eth_rx_queue_setup(
			l->config.link_id,
			i,
			l->config.rx.queue_sz,
			l->config.numa_node,
			NULL,
			l->config.rx.mp);
		if (rc < 0) {
                        rc = -rte_errno;
			goto err;
		}
	}

	/* Port TX */
	for (i = 0; i < l->config.tx.nb_queues; i++) {
		rc = rte_eth_tx_queue_setup(
			l->config.link_id,
			i,
			l->config.tx.queue_sz,
			l->config.numa_node,
			NULL);
		if (rc < 0) {
                        rc = -rte_errno;
			goto err;
		}
	}

	TAILQ_INSERT_TAIL(&link_node, l, next);
        return 0;

err:
	if (l)
		rte_free(l);
        return rc;
}

int
link_config_rem(char const *name)
{
        struct link *l = link_config_get(name);
        if (l) {
                TAILQ_REMOVE(&link_node, l, next);
		rte_free(l);
                return 0;
        }

        return -ENOENT;
}

int
link_config_set_promiscuous(char const *name, bool enable)
{
	struct link *l = link_config_get(name);
	uint16_t port_id = 0;
	int rc = -EINVAL;

        if (l) {
		port_id = l->config.link_id;
		rc = enable ?
			rte_eth_promiscuous_enable(port_id) :
			rte_eth_promiscuous_disable(port_id);
		if (rc < 0)
			return rc;

		l->config.promiscuous = enable;
	}

	return rc;
}

int
link_config_set_mtu(char const *name, uint32_t mtu)
{
	struct link *l = link_config_get(name);
	uint16_t port_id = 0;
	int rc = -EINVAL;

        if (l) {
		port_id = l->config.link_id;
		rc = rte_eth_dev_set_mtu(port_id, mtu);
		if (rc < 0)
			return rc;
		l->config.mtu = mtu;
	}

	return rc;
}

int
link_config_set_peer(char const *name, char const *peer_name)
{
	struct link *peer = link_config_get(peer_name);
	struct link *l = link_config_get(name);
	int rc = -ENOENT;

        if (l && peer) {
		if (l->config.peer.link_id == LINK_ID_MAX) {
			strncpy(l->config.peer.link_name, peer->config.link_name, RTE_ETH_NAME_MAX_LEN);
			l->config.peer.link_id = peer->config.link_id;
		} else {
			rc = -EINVAL;
		}
		if (peer->config.peer.link_id == LINK_ID_MAX) {
			strncpy(peer->config.peer.link_name, l->config.link_name, RTE_ETH_NAME_MAX_LEN);
			peer->config.peer.link_id = l->config.link_id;
		} else {
			rc = -EINVAL;
		}

		return 0;
	}

	return rc;
}

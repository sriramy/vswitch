/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdint.h>
#include <linux/if.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>
#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_num.h>
#include <cmdline_rdline.h>

#include "cli.h"
#include "cli_link.h"
#include "link.h"

static const char
cmd_link_show_help[] = "link show";

static const char
cmd_link_dev_show_help[] = "link <dev> show";

static const char
cmd_link_config_help[] = "link <dev> config rxq <nb_rxq> txq <nb_txq> mempool <mp_name>";

static struct link_head link_node = TAILQ_HEAD_INITIALIZER(link_node);

static struct link*
link_config_get(char const *name)
{
	struct link *l;

	TAILQ_FOREACH(l, &link_node, next) {
		if (strcmp(l->config.link_name, name) == 0)
			return l;
	}
	return NULL;
}

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

static void
cli_link_config(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_config_cmd_tokens *res = parsed_result;
        char link_name[RTE_ETH_NAME_MAX_LEN];
	struct rte_eth_dev_info link_info;
	struct rte_eth_conf link_conf;
	int rc = -EINVAL;
	uint16_t link_id;
	struct link* l;
	int numa_node;
	uint32_t i;

	rc = rte_eth_dev_get_port_by_name(res->dev, &link_id);
	if (rc < 0) {
                cmdline_printf(cl, "%s: rte_eth_dev_get_port_by_name failed: %s\n", res->dev, rte_strerror(rte_errno));
		goto err;
	}

	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

	l = link_config_get(link_name);
	if (!l) {
		l = rte_malloc(NULL, sizeof(struct link), 0);
		if (!l) {
                        cmdline_printf(cl, "%s\n", rte_strerror(rte_errno));
                        goto err;
                }
	} else {
                cmdline_printf(cl, MSG_ERR_EXIST);
		goto err;
        }

	memset(&l->config, 0, sizeof(struct link_config));
	l->config.rx.nb_queues = res->nb_rxq;
	l->config.rx.queue_sz = ETHDEV_RX_DESC_DEFAULT;

	l->config.tx.nb_queues = res->nb_txq;
	l->config.tx.queue_sz = ETHDEV_TX_DESC_DEFAULT;

	l->config.mtu = link_conf_default.rxmode.mtu;


	rc = rte_eth_dev_info_get(link_id, &link_info);
	if (rc) {
                cmdline_printf(cl, "%s: %s\n", link_name, rte_strerror(rte_errno));
		goto err;
	}

	l->config.rx.mp = rte_mempool_lookup(res->mp_name);
	if (!l->config.rx.mp) {
                cmdline_printf(cl, "%s: rte_mempool_lookup failed: %s\n", link_name, rte_strerror(rte_errno));
		goto err;
	}

	memcpy(&link_conf, &link_conf_default, sizeof(struct rte_eth_conf));
	numa_node = rte_eth_dev_socket_id(link_id);
	if (numa_node == SOCKET_ID_ANY)
		numa_node = 0;

	rc = rte_eth_dev_configure(
		link_id,
		l->config.rx.nb_queues,
		l->config.tx.nb_queues,
		&link_conf);
	if (rc < 0) {
                cmdline_printf(cl, "%s: rte_eth_dev_configure failed: %s\n", link_name, rte_strerror(rte_errno));
		goto err;
	}

	/* Port RX */
	for (i = 0; i < l->config.rx.nb_queues; i++) {
		rc = rte_eth_rx_queue_setup(
			link_id,
			i,
			l->config.rx.queue_sz,
			numa_node,
			NULL,
			l->config.rx.mp);
		if (rc < 0) {
			cmdline_printf(cl, "%s: rte_eth_rx_queue_setup failed: %s\n", link_name, rte_strerror(rte_errno));
			goto err;
		}
	}

	/* Port TX */
	for (i = 0; i < l->config.tx.nb_queues; i++) {
		rc = rte_eth_tx_queue_setup(
			link_id,
			i,
			l->config.tx.queue_sz,
			numa_node,
			NULL);
		if (rc < 0) {
			cmdline_printf(cl, "%s: rte_eth_tx_queue_setup failed: %s\n", link_name, rte_strerror(rte_errno));
			goto err;
		}
	}

	TAILQ_INSERT_TAIL(&link_node, l, next);
	return;

err:
        cmdline_printf(cl, MSG_CMD_FAIL, "link");
}

cmdline_parse_token_string_t link_config_cmd =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, cmd, "link");
cmdline_parse_token_string_t link_config_dev =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, dev, NULL);
cmdline_parse_token_string_t link_config_config =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, cmd, "config");
cmdline_parse_token_string_t link_config_rxq =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, rxq, "rxq");
cmdline_parse_token_num_t link_config_nb_rxq =
	TOKEN_NUM_INITIALIZER(struct link_config_cmd_tokens, nb_rxq, RTE_UINT16);
cmdline_parse_token_string_t link_config_txq =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, txq, "txq");
cmdline_parse_token_num_t link_config_nb_txq =
	TOKEN_NUM_INITIALIZER(struct link_config_cmd_tokens, nb_txq, RTE_UINT16);
cmdline_parse_token_string_t link_config_mempool =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, mempool, "mempool");
cmdline_parse_token_string_t link_config_mp_name =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, mp_name, NULL);

cmdline_parse_inst_t link_config_cmd_ctx = {
	.f = cli_link_config,
	.data = NULL,
	.help_str = cmd_link_config_help,
	.tokens = {
		(void *)&link_config_cmd,
		(void *)&link_config_dev,
		(void *)&link_config_config,
		(void *)&link_config_rxq,
		(void *)&link_config_nb_rxq,
		(void *)&link_config_txq,
		(void *)&link_config_nb_txq,
		(void *)&link_config_mempool,
		(void *)&link_config_mp_name,
		NULL,
	},
};

static int
link_show_port(struct cmdline *cl, uint16_t port_id)
{
       	uint16_t mtu = 0;
	struct rte_eth_dev_info info;
	struct rte_eth_stats stats;
	struct rte_ether_addr addr;
	struct rte_eth_link link;
        char name[IFNAMSIZ];
	int rc;

	rc = rte_eth_dev_get_name_by_port(port_id, name);
	if (rc < 0)
		return rc;

	rte_eth_dev_info_get(port_id, &info);
	rte_eth_stats_get(port_id, &stats);
	rte_eth_macaddr_get(port_id, &addr);
	rte_eth_link_get(port_id, &link);
	rte_eth_dev_get_mtu(port_id, &mtu);

	cmdline_printf(cl,
		 "%s: flags=<%s> mtu %u\n"
		 "\tether " RTE_ETHER_ADDR_PRT_FMT " rxq %u txq %u\n"
		 "\tport# %u  speed %s\n"
		 "\tRX packets %" PRIu64"  bytes %" PRIu64"\n"
		 "\tRX errors %" PRIu64"  missed %" PRIu64"  no-mbuf %" PRIu64"\n"
		 "\tTX packets %" PRIu64"  bytes %" PRIu64"\n"
		 "\tTX errors %" PRIu64"\n\n",
		 &name[0],
		 link.link_status ? "UP" : "DOWN",
		 mtu,
		 RTE_ETHER_ADDR_BYTES(&addr),
		 info.nb_rx_queues,
		 info.nb_tx_queues,
		 port_id,
		 rte_eth_link_speed_to_str(link.link_speed),
		 stats.ipackets,
		 stats.ibytes,
		 stats.ierrors,
		 stats.imissed,
		 stats.rx_nombuf,
		 stats.opackets,
		 stats.obytes,
		 stats.oerrors);

	return 0;
}

static void
cli_link_dev_show(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_show_cmd_tokens *res = parsed_result;
	int rc = -EINVAL;
	uint16_t port_id;

	rc = rte_eth_dev_get_port_by_name(res->dev, &port_id);
	if (rc < 0)
		return;

	link_show_port(cl, port_id);
}

static void
cli_link_show(__rte_unused void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
        uint16_t port_id;
        RTE_ETH_FOREACH_DEV(port_id) {
                link_show_port(cl, port_id);
        }
}

cmdline_parse_token_string_t link_show_cmd =
	TOKEN_STRING_INITIALIZER(struct link_show_cmd_tokens, cmd, "link");
cmdline_parse_token_string_t link_show_show =
	TOKEN_STRING_INITIALIZER(struct link_show_cmd_tokens, show, "show");
cmdline_parse_token_string_t link_show_dev =
	TOKEN_STRING_INITIALIZER(struct link_show_cmd_tokens, dev, NULL);

cmdline_parse_inst_t link_show_cmd_ctx = {
	.f = cli_link_show,
	.data = NULL,
	.help_str = cmd_link_show_help,
	.tokens = {
		(void *)&link_show_cmd,
		(void *)&link_show_show,
		NULL,
	},
};

cmdline_parse_inst_t link_dev_show_cmd_ctx = {
	.f = cli_link_dev_show,
	.data = NULL,
	.help_str = cmd_link_dev_show_help,
	.tokens = {
		(void *)&link_show_cmd,
		(void *)&link_show_show,
		(void *)&link_show_dev,
		NULL,
	},
};

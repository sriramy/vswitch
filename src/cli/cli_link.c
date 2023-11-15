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
cmd_link_dev_config_add_help[] = "link <dev> config add rxq <nb_rxq> txq <nb_txq> mempool <mp_name>";

static const char
cmd_link_dev_config_rem_help[] = "link <dev> config rem";

static const char
cmd_link_dev_config_show_help[] = "link <dev> config show";

static const char
cmd_link_dev_config_set_promiscuous_help[] = "link <dev> config promiscuous <on#off>";

static const char
cmd_link_dev_config_set_mtu_help[] = "link <dev> config mtu <mtu>";

static void
cli_link_dev_config_add(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_config_cmd_tokens *res = parsed_result;
	struct link_config config;
	int rc;

	memset(&config, 0, sizeof(config));

	rte_strscpy(config.link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	config.link_name[strlen(res->dev)] = '\0';
	config.rx.nb_queues = res->nb_rxq;
	config.rx.queue_sz = ETHDEV_RX_DESC_DEFAULT;
	rte_strscpy(config.rx.mp_name, res->mp_name, RTE_MEMPOOL_NAMESIZE);
	config.rx.mp_name[strlen(res->mp_name)] = '\0';

	config.tx.nb_queues = res->nb_txq;
	config.tx.queue_sz = ETHDEV_TX_DESC_DEFAULT;
	rc = link_config_add(&config);
	if (rc < 0) {
                cmdline_printf(cl, "link %s config add failed: %s\n", config.link_name, rte_strerror(-rc));
	}
}

static void
cli_link_dev_config_rem(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_config_cmd_tokens *res = parsed_result;
	char link_name[RTE_ETH_NAME_MAX_LEN];
	int rc;

	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

	rc = link_config_rem(link_name);
	if (rc < 0) {
                cmdline_printf(cl, "link %s config rem failed: %s\n", link_name, rte_strerror(-rc));
	}
}

static void
cli_link_dev_config_show(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_config_cmd_tokens *res = parsed_result;
	char link_name[RTE_ETH_NAME_MAX_LEN];
	int rc = -ENOENT;
	struct link* l;

	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

	l = link_config_get(link_name);
	if (!l) {
                cmdline_printf(cl, "link %s config show failed: %s\n", link_name, rte_strerror(-rc));
	} else {
		cmdline_printf(cl,
			"%s: link_id=<%u> numa %d\n"
			"\t rxq %u size %d mempool %s\n"
			"\t txq %u size %d\n"
			"\t promiscuous %d mtu %u\n",
			l->config.link_name,
			l->config.link_id,
			l->config.numa_node,
			l->config.rx.nb_queues,
			l->config.rx.queue_sz,
			l->config.rx.mp_name,
			l->config.tx.nb_queues,
			l->config.tx.queue_sz,
			l->config.promiscuous,
			l->config.mtu);
	}
}

static void
cli_link_dev_config_set_promiscuous(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_config_cmd_tokens *res = parsed_result;
	char link_name[RTE_ETH_NAME_MAX_LEN];
	bool enable = false;
	int rc = -ENOENT;

	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

	if (!strcmp(res->promiscuous, "on"))
		enable = true;

	rc = link_config_set_promiscuous(link_name, enable);
	if (rc < 0) {
                cmdline_printf(cl, "link %s config promiscuous failed: %s\n", link_name, rte_strerror(-rc));
	}
}

static void
cli_link_dev_config_set_mtu(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct link_config_cmd_tokens *res = parsed_result;
	char link_name[RTE_ETH_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

	rc = link_config_set_mtu(link_name, res->mtu);
	if (rc < 0) {
                cmdline_printf(cl, "link %s config mtu failed: %s\n", link_name, rte_strerror(-rc));
	}
}

cmdline_parse_token_string_t link_dev_config_cmd =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, cmd, "link");
cmdline_parse_token_string_t link_dev_config_dev =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, dev, NULL);
cmdline_parse_token_string_t link_dev_config_config =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, config, "config");
cmdline_parse_token_string_t link_dev_config_add =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, action, "add");
cmdline_parse_token_string_t link_dev_config_rem =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, action, "rem");
cmdline_parse_token_string_t link_dev_config_show =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, action, "show");
cmdline_parse_token_string_t link_dev_config_set_promiscuous =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, action, "promiscuous");
cmdline_parse_token_string_t link_dev_config_promiscuous =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, promiscuous, "on#off");
cmdline_parse_token_string_t link_dev_config_set_mtu =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, action, "mtu");
cmdline_parse_token_num_t link_dev_config_mtu =
	TOKEN_NUM_INITIALIZER(struct link_config_cmd_tokens, mtu, RTE_UINT16);
cmdline_parse_token_string_t link_dev_config_rxq =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, rxq, "rxq");
cmdline_parse_token_num_t link_dev_config_nb_rxq =
	TOKEN_NUM_INITIALIZER(struct link_config_cmd_tokens, nb_rxq, RTE_UINT16);
cmdline_parse_token_string_t link_dev_config_txq =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, txq, "txq");
cmdline_parse_token_num_t link_dev_config_nb_txq =
	TOKEN_NUM_INITIALIZER(struct link_config_cmd_tokens, nb_txq, RTE_UINT16);
cmdline_parse_token_string_t link_dev_config_mempool =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, mempool, "mempool");
cmdline_parse_token_string_t link_dev_config_mp_name =
	TOKEN_STRING_INITIALIZER(struct link_config_cmd_tokens, mp_name, NULL);

cmdline_parse_inst_t link_dev_config_add_cmd_ctx = {
	.f = cli_link_dev_config_add,
	.data = NULL,
	.help_str = cmd_link_dev_config_add_help,
	.tokens = {
		(void *)&link_dev_config_cmd,
		(void *)&link_dev_config_dev,
		(void *)&link_dev_config_config,
		(void *)&link_dev_config_add,
		(void *)&link_dev_config_rxq,
		(void *)&link_dev_config_nb_rxq,
		(void *)&link_dev_config_txq,
		(void *)&link_dev_config_nb_txq,
		(void *)&link_dev_config_mempool,
		(void *)&link_dev_config_mp_name,
		NULL,
	},
};

cmdline_parse_inst_t link_dev_config_rem_cmd_ctx = {
	.f = cli_link_dev_config_rem,
	.data = NULL,
	.help_str = cmd_link_dev_config_rem_help,
	.tokens = {
		(void *)&link_dev_config_cmd,
		(void *)&link_dev_config_dev,
		(void *)&link_dev_config_config,
		(void *)&link_dev_config_rem,
		NULL,
	},
};

cmdline_parse_inst_t link_dev_config_show_cmd_ctx = {
	.f = cli_link_dev_config_show,
	.data = NULL,
	.help_str = cmd_link_dev_config_show_help,
	.tokens = {
		(void *)&link_dev_config_cmd,
		(void *)&link_dev_config_dev,
		(void *)&link_dev_config_config,
		(void *)&link_dev_config_show,
		NULL,
	},
};

cmdline_parse_inst_t link_dev_config_set_promiscuous_cmd_ctx = {
	.f = cli_link_dev_config_set_promiscuous,
	.data = NULL,
	.help_str = cmd_link_dev_config_set_promiscuous_help,
	.tokens = {
		(void *)&link_dev_config_cmd,
		(void *)&link_dev_config_dev,
		(void *)&link_dev_config_config,
		(void *)&link_dev_config_set_promiscuous,
		(void *)&link_dev_config_promiscuous,
		NULL,
	},
};

cmdline_parse_inst_t link_dev_config_set_mtu_cmd_ctx = {
	.f = cli_link_dev_config_set_mtu,
	.data = NULL,
	.help_str = cmd_link_dev_config_set_mtu_help,
	.tokens = {
		(void *)&link_dev_config_cmd,
		(void *)&link_dev_config_dev,
		(void *)&link_dev_config_config,
		(void *)&link_dev_config_set_mtu,
		(void *)&link_dev_config_mtu,
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
cmdline_parse_token_string_t link_show_dev =
	TOKEN_STRING_INITIALIZER(struct link_show_cmd_tokens, dev, NULL);
cmdline_parse_token_string_t link_show_show =
	TOKEN_STRING_INITIALIZER(struct link_show_cmd_tokens, show, "show");

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
		(void *)&link_show_dev,
		(void *)&link_show_show,
		NULL,
	},
};

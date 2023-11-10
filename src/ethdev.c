/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdint.h>
#include <linux/if.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_rdline.h>

struct ethdev_show_cmd_tokens {
	cmdline_fixed_string_t cmd;
	cmdline_fixed_string_t show;
};

static const char
cmd_ethdev_show_help[] = "ethdev show";

static int
ethdev_show_port(struct cmdline *cl, uint16_t port_id)
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
		 "\tether " RTE_ETHER_ADDR_PRT_FMT " rxqueues %u txqueues %u\n"
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
cli_ethdev_show(__rte_unused void *parsed_result, struct cmdline *cl, void *data __rte_unused)
{
        uint16_t port_id;
        RTE_ETH_FOREACH_DEV(port_id) {
                ethdev_show_port(cl, port_id);
        }
}

cmdline_parse_token_string_t ethdev_cmd =
	TOKEN_STRING_INITIALIZER(struct ethdev_show_cmd_tokens, cmd, "ethdev");
cmdline_parse_token_string_t ethdev_show =
	TOKEN_STRING_INITIALIZER(struct ethdev_show_cmd_tokens, show, "show");

cmdline_parse_inst_t ethdev_show_cmd_ctx = {
	.f = cli_ethdev_show,
	.data = NULL,
	.help_str = cmd_ethdev_show_help,
	.tokens = {
		(void *)&ethdev_cmd,
		(void *)&ethdev_show,
		NULL,
	},
};

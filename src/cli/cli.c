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
#include <cmdline_socket.h>

#include "cli.h"
#include "cli_link.h"
#include "cli_mempool.h"
#include "cli_stage.h"
#include "cli_vswitch.h"

static struct cmdline *cl;

static volatile bool stopped = false;

static char const
cmd_quit_help[] = "quit";

static void
cli_quit_cmd(__rte_unused void *parsed_result, __rte_unused struct cmdline *cl, __rte_unused void *data)
{
	cli_quit();
}

cmdline_parse_token_string_t quit_cmd =
	TOKEN_STRING_INITIALIZER(struct quit_cmd_tokens, cmd, "quit");

cmdline_parse_inst_t quit_cmd_ctx = {
	.f = cli_quit_cmd,
	.data = NULL,
	.help_str = cmd_quit_help,
	.tokens = {
		(void *)&quit_cmd,
		NULL,
	},
};

cmdline_parse_ctx_t commands_ctx[] = {
	(cmdline_parse_inst_t *)&quit_cmd_ctx,

	(cmdline_parse_inst_t *)&link_show_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_show_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_add_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_rem_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_show_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_set_promiscuous_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_set_mtu_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_set_peer_cmd_ctx,

	(cmdline_parse_inst_t *)&mempool_add_cmd_ctx,
	(cmdline_parse_inst_t *)&mempool_rem_show_cmd_ctx,

	(cmdline_parse_inst_t *)&stage_add_cmd_ctx,
	(cmdline_parse_inst_t *)&stage_rem_cmd_ctx,
	(cmdline_parse_inst_t *)&stage_show_cmd_ctx,

	(cmdline_parse_inst_t *)&vswitch_show_cmd_ctx,

	NULL,
};

int
cli_init(char const *prompt)
{
	cl = cmdline_stdin_new(commands_ctx, prompt);
	if (!cl) {
		return -ENOMEM;
	}

	return 0;
}

void
cli_interact()
{
	cmdline_interact(cl);
}

bool
cli_stopped()
{
	return stopped;
}

void
cli_quit()
{
	stopped = true;
	cmdline_stdin_exit(cl);
}
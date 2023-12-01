/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>
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

cmdline_parse_ctx_t commands_ctx[] = {
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
	(cmdline_parse_inst_t *)&stage_set_type_cmd_ctx,
	(cmdline_parse_inst_t *)&stage_set_queue_in_cmd_ctx,
	(cmdline_parse_inst_t *)&stage_set_queue_out_cmd_ctx,

	(cmdline_parse_inst_t *)&vswitch_show_cmd_ctx,
	(cmdline_parse_inst_t *)&vswitch_start_cmd_ctx,

	NULL,
};

static int
is_comment(char *in)
{
	if ((strlen(in) && index("!#%;", in[0])) ||
		(strncmp(in, "//", 2) == 0) ||
		(strncmp(in, "--", 2) == 0))
		return 1;

	return 0;
}

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

void
cli_quit()
{
	cmdline_stdin_exit(cl);
}

int
cli_execute(const char *file_name)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int rc;

	fp = fopen(file_name, "r");
	if (fp == NULL)
		return -ENOENT;

	while ((read = getline(&line, &len, fp)) != -1) {
		cmdline_printf(cl, "%s", line);
		if (is_comment(line))
			continue;

		rc = cmdline_parse(cl, line);
		if (rc == CMDLINE_PARSE_AMBIGUOUS)
			cmdline_printf(cl, "Ambiguous command\n");
		else if (rc == CMDLINE_PARSE_NOMATCH)
			cmdline_printf(cl, "Command not found\n");
		else if (rc == CMDLINE_PARSE_BAD_ARGS)
			cmdline_printf(cl, "Bad arguments\n");
	}

	free(line);
	fclose(fp);
	return 0;
}

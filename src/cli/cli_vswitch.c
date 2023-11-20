/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>

#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_num.h>

#include "cli.h"
#include "cli_vswitch.h"
#include "stage.h"

static char const
cmd_vswitch_show_help[] = "vswitch show";

static void
cli_vswitch_show(__rte_unused void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
        cmdline_printf(cl, "enabled_coremask: 0x%04lx\n",
                stage_get_enabled_coremask());
        cmdline_printf(cl, "used_coremask: 0x%04lx\n",
                stage_get_used_coremask());
}

cmdline_parse_token_string_t vswitch_cmd =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, cmd, "vswitch");
cmdline_parse_token_string_t vswitch_show =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, action, "show");

cmdline_parse_inst_t vswitch_show_cmd_ctx = {
	.f = cli_vswitch_show,
	.data = NULL,
	.help_str = cmd_vswitch_show_help,
	.tokens = {
		(void *)&vswitch_cmd,
                (void *)&vswitch_show,
		NULL,
	},
};

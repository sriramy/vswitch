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
#include "link.h"
#include "stage.h"
#include "vswitch.h"

static void
cli_vswitch_show(__rte_unused void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct vswitch_config *config = vswitch_config_get();
	cmdline_printf(cl, "Vswitch\n");
	if (!config) {
		cmdline_printf(cl, "  Not available\n");
	} else {
		cmdline_printf(cl, "  Event device: %d\n", config->eventdev_id);
		cmdline_printf(cl, "  Driver: %s\n", config->eventdev_info.driver_name);
	}
	cmdline_printf(cl, "Stage\n");
        cmdline_printf(cl, "  enabled_coremask: 0x%04lx\n",
                stage_get_enabled_coremask());
        cmdline_printf(cl, "  used_coremask: 0x%04lx\n",
                stage_get_used_coremask());
}

static void
cli_vswitch_start(__rte_unused void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
        cmdline_printf(cl, "Starting vswitch...\n");
        vswitch_start();
        cmdline_printf(cl, "Done.\n");
}

cmdline_parse_token_string_t vswitch_cmd =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, cmd, "vswitch");
cmdline_parse_token_string_t vswitch_action_show =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, action, "show");
cmdline_parse_token_string_t vswitch_action_start =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, action, "start");

cmdline_parse_inst_t vswitch_show_cmd_ctx = {
	.f = cli_vswitch_show,
	.data = NULL,
	.help_str = "vswitch show",
	.tokens = {
		(void *)&vswitch_cmd,
                (void *)&vswitch_action_show,
		NULL,
	},
};

cmdline_parse_inst_t vswitch_start_cmd_ctx = {
	.f = cli_vswitch_start,
	.data = NULL,
	.help_str = "vswitch start",
	.tokens = {
		(void *)&vswitch_cmd,
                (void *)&vswitch_action_start,
		NULL,
	},
};

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
	struct lcore_params *lcore;
	uint16_t core_id;

	cmdline_printf(cl, "Vswitch\n");
	if (!config) {
		cmdline_printf(cl, "  Not initialized\n");
		return;
	}

	cmdline_printf(cl, "  Enabled worker cores:\t0x%04lx\n",
		stage_get_enabled_coremask());
	cmdline_printf(cl, "  Used worker cores:\t0x%04lx\n",
		stage_get_used_coremask());
	cmdline_printf(cl, "  Event device,\tid: %d, \tdriver: %s\n",
			config->ev_id,
			config->ev_info.driver_name);
	cmdline_printf(cl, "  Number of event ports:\t%d\n", config->nb_ports);
	cmdline_printf(cl, "  Number of event queues:\t%d\n", config->nb_queues);
	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		lcore = &config->lcores[core_id];
		if (!lcore->enabled) {
			if (core_id == rte_get_main_lcore())
				cmdline_printf(cl, "Main\tlcore %u\n", core_id);
			else if (rte_lcore_has_role(core_id, ROLE_SERVICE))
				cmdline_printf(cl, "Service\tlcore %u\n", core_id);
			else
				cmdline_printf(cl, "Lcore %u disabled\n", core_id);
		} else {
			cmdline_printf(cl, "Worker\tlcore %u\ttype: %s\t"
				"ev_id: %u,\tev_port_id: %u\t"
				"ev_in_queue: %u\tev_out_queue: %u"
				"nb_link_queues: %u\n",
				core_id, stage_type_str[lcore->type],
				lcore->ev_id, lcore->ev_port_id,
				lcore->ev_in_queue, lcore->ev_out_queue,
				lcore->nb_link_queues);
		}
	}
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

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
				"ev_in_queue: %u\tev_out_queue: %u\t"
				"nb_link_in_queues: %u\tnb_link_out_queues: %u\t"
				"graph: %s\n",
				core_id, stage_type_str[lcore->type],
				lcore->ev_id, lcore->ev_port_id,
				lcore->ev_in_queue, lcore->ev_out_queue,
				lcore->nb_link_in_queues, lcore->nb_link_out_queues,
				lcore->graph_name);
			rte_graph_dump(stdout, lcore->graph_id);
		}
	}
}

static void
cli_vswitch_start(__rte_unused void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
        int rc;
	
	cmdline_printf(cl, "Starting vswitch...\n");
        rc = vswitch_start();
	if (rc < 0)
                cmdline_printf(cl, "Vswitch start failed: %s\n", rte_strerror(-rc));
	else
		cmdline_printf(cl, "Done.\n");
}

static void
cli_vswitch_stats(__rte_unused void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int rc;

        rc = vswitch_dump_stats(TMP_STATS_FILE);
	if (rc < 0) {
                cmdline_printf(cl, "Vswitch stats failed: %s\n", rte_strerror(-rc));
		return;
	}

	fp = fopen(TMP_STATS_FILE, "r");
	if (fp) {
		while ((read = getline(&line, &len, fp)) != -1) {
			cmdline_printf(cl, "%s", line);
		}
	} else {
		cmdline_printf(cl, "Vswitch stats file open failed: %s\n", rte_strerror(-rte_errno));
	}

	free(line);
	fclose(fp);
}

cmdline_parse_token_string_t vswitch_cmd =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, cmd, "vswitch");
cmdline_parse_token_string_t vswitch_action_show =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, action, "show");
cmdline_parse_token_string_t vswitch_action_start =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, action, "start");
cmdline_parse_token_string_t vswitch_action_stats =
	TOKEN_STRING_INITIALIZER(struct vswitch_cmd_tokens, action, "stats");

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

cmdline_parse_inst_t vswitch_stats_cmd_ctx = {
	.f = cli_vswitch_stats,
	.data = NULL,
	.help_str = "vswitch stats",
	.tokens = {
		(void *)&vswitch_cmd,
                (void *)&vswitch_action_stats,
		NULL,
	},
};

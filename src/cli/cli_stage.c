/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_eventdev.h>
#include <rte_malloc.h>

#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_num.h>

#include "cli.h"
#include "cli_stage.h"
#include "stage.h"

static void
cli_stage_add(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        struct stage_config config;
	int rc;

	memset(&config, 0, sizeof(config));

	rte_strscpy(config.name, res->name, STAGE_NAME_MAX_LEN);
	config.name[strlen(res->name)] = '\0';
	config.coremask = res->mask;
	config.type = STAGE_TYPE_MAX;

	rc = stage_config_add(&config);
	if (rc < 0) {
                cmdline_printf(cl, "stage add %s failed: %s\n", config.name, rte_strerror(-rc));
        }
}

static void
cli_stage_rem(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
        int rc = -EINVAL;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';
	rc = stage_config_rem(stage_name);
	if (rc < 0) {
                cmdline_printf(cl, "stage rem %s failed: %s\n", stage_name, rte_strerror(-rc));
	}
}

static void
cli_stage_show(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
        struct stage *stage;
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

        stage = stage_config_get(stage_name);
        if (!stage) {
                cmdline_printf(cl, "stage show %s failed: %s\n", stage_name, rte_strerror(-rc));
        } else {
                cmdline_printf(cl,
                        "%s: stage_id=%d coremask:0x%04x\n",
                        stage->config.name, stage->config.stage_id,
			stage->config.coremask);
        }
}

static void
cli_stage_set_type(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
	uint8_t stage_type = STAGE_TYPE_MAX;
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

	if (strcmp(res->stage_type, "rx") == 0)
		stage_type = STAGE_TYPE_RX;
	else if (strcmp(res->stage_type, "worker") == 0)
		stage_type = STAGE_TYPE_WORKER;
	else if (strcmp(res->stage_type, "tx") == 0)
		stage_type = STAGE_TYPE_TX;

	rc = stage_config_set_type(stage_name, stage_type);
	if (rc < 0) {
		cmdline_printf(cl, "stage set %s type %s failed: %s\n",
			       stage_name, res->stage_type, rte_strerror(-rc));
	}
}

static void
cli_stage_set_queue_in(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

        rc = stage_config_set_ev_queue_in(stage_name,
					  res->in_qid,
					  (strcmp(res->schedule_type, "atomic") == 0) ?
						RTE_SCHED_TYPE_ATOMIC :
					  	RTE_SCHED_TYPE_ORDERED,
					  res->mp_name);
        if (rc < 0)
                cmdline_printf(cl, "stage set %s input event queue failed: %s\n",
			       stage_name, rte_strerror(-rc));
}

static void
cli_stage_set_queue_out(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

        rc = stage_config_set_ev_queue_out(stage_name,
					  res->out_qid,
					  (strcmp(res->schedule_type, "atomic") == 0) ?
						RTE_SCHED_TYPE_ATOMIC :
					  	RTE_SCHED_TYPE_ORDERED);
        if (rc < 0)
                cmdline_printf(cl, "stage set %s output event queue failed: %s\n",
			       stage_name, rte_strerror(-rc));
}

static void
cli_stage_set_link_queue_in(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
        char link_name[RTE_ETH_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';
	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

        rc = stage_config_set_link_queue_in(stage_name, link_name, res->in_qid);
        if (rc < 0)
                cmdline_printf(cl, "stage set %s input link queue failed: %s\n",
			       stage_name, rte_strerror(-rc));
}

static void
cli_stage_set_link_queue_out(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
        char link_name[RTE_ETH_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';
	rte_strscpy(link_name, res->dev, RTE_ETH_NAME_MAX_LEN);
	link_name[strlen(res->dev)] = '\0';

        rc = stage_config_set_link_queue_out(stage_name, link_name, res->out_qid);
        if (rc < 0)
                cmdline_printf(cl, "stage set %s output link queue failed: %s\n",
			       stage_name, rte_strerror(-rc));
}

static void
cli_stage_set_graph_nodes(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        char stage_name[STAGE_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

        rc = stage_config_set_graph_nodes(stage_name, res->nodes);
        if (rc < 0)
                cmdline_printf(cl, "stage set %s output link queue failed: %s\n",
			       stage_name, rte_strerror(-rc));
}

cmdline_parse_token_string_t stage_cmd =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, stage, "stage");
cmdline_parse_token_string_t stage_add =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "add");
cmdline_parse_token_string_t stage_rem =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "rem");
cmdline_parse_token_string_t stage_show =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "show");
cmdline_parse_token_string_t stage_set =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "set");
cmdline_parse_token_string_t stage_name =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, name, NULL);
cmdline_parse_token_string_t stage_coremask =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, coremask, "coremask");
cmdline_parse_token_num_t stage_mask =
	TOKEN_NUM_INITIALIZER(struct stage_cmd_tokens, mask, RTE_UINT32);
cmdline_parse_token_string_t stage_type =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, type, "type");
cmdline_parse_token_string_t stage_stage_type =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, stage_type, "rx#worker#tx");
cmdline_parse_token_string_t stage_queue =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, stage_queue, "queue");
cmdline_parse_token_string_t stage_in_queue =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, in_queue, "in");
cmdline_parse_token_num_t stage_in_qid =
	TOKEN_NUM_INITIALIZER(struct stage_cmd_tokens, in_qid, RTE_UINT32);
cmdline_parse_token_string_t stage_out_queue =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, out_queue, "out");
cmdline_parse_token_num_t stage_out_qid =
	TOKEN_NUM_INITIALIZER(struct stage_cmd_tokens, out_qid, RTE_UINT32);
cmdline_parse_token_string_t stage_schedule =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, schedule, "schedule");
cmdline_parse_token_string_t stage_schedule_type =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, schedule_type, "atomic#ordered");
cmdline_parse_token_string_t stage_mempool =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, mempool, "mempool");
cmdline_parse_token_string_t stage_mp_name =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, mp_name, NULL);
cmdline_parse_token_string_t stage_link =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, link, "link");
cmdline_parse_token_string_t stage_dev =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, dev, NULL);
cmdline_parse_token_string_t stage_graph =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, graph, "graph");
cmdline_parse_token_string_t stage_nodes =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, nodes, NULL);

static char const
cmd_stage_add_help[] = "stage add <stage_name> [coremask <mask>]";

cmdline_parse_inst_t stage_add_cmd_ctx = {
	.f = cli_stage_add,
	.data = NULL,
	.help_str = cmd_stage_add_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_add,
		(void *)&stage_name,
		(void *)&stage_coremask,
		(void *)&stage_mask,
		NULL,
	},
};

static char const
cmd_stage_rem_help[] = "stage rem <stage_name>";

cmdline_parse_inst_t stage_rem_cmd_ctx = {
	.f = cli_stage_rem,
	.data = NULL,
	.help_str = cmd_stage_rem_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_rem,
		(void *)&stage_name,
		NULL,
	},
};

static char const
cmd_stage_show_help[] = "stage show <stage_name>";

cmdline_parse_inst_t stage_show_cmd_ctx = {
	.f = cli_stage_show,
	.data = NULL,
	.help_str = cmd_stage_show_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_show,
		(void *)&stage_name,
		NULL,
	},
};

static char const
cmd_stage_set_type_help[] = "stage set <stage_name> type rx#worker#tx";

cmdline_parse_inst_t stage_set_type_cmd_ctx = {
	.f = cli_stage_set_type,
	.data = NULL,
	.help_str = cmd_stage_set_type_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_type,
		(void *)&stage_stage_type,
		NULL,
	},
};

static char const
cmd_stage_set_queue_in_help[] = "stage set <stage_name> queue in <qid> schedule <atomic#ordered> mempool <mp_name>";

cmdline_parse_inst_t stage_set_queue_in_cmd_ctx = {
	.f = cli_stage_set_queue_in,
	.data = NULL,
	.help_str = cmd_stage_set_queue_in_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_queue,
		(void *)&stage_in_queue,
		(void *)&stage_in_qid,
		(void *)&stage_schedule,
		(void *)&stage_schedule_type,
		(void *)&stage_mempool,
		(void *)&stage_mp_name,
		NULL,
	},
};

static char const
cmd_stage_set_queue_out_help[] = "stage set <stage_name> queue out <qid> schedule <atomic#ordered>";

cmdline_parse_inst_t stage_set_queue_out_cmd_ctx = {
	.f = cli_stage_set_queue_out,
	.data = NULL,
	.help_str = cmd_stage_set_queue_out_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_queue,
		(void *)&stage_out_queue,
		(void *)&stage_out_qid,
		(void *)&stage_schedule,
		(void *)&stage_schedule_type,
		NULL,
	},
};

static char const
cmd_stage_set_link_queue_in_help[] = "stage set <stage_name> link <name> queue in <qid> ";

cmdline_parse_inst_t stage_set_link_queue_in_cmd_ctx = {
	.f = cli_stage_set_link_queue_in,
	.data = NULL,
	.help_str = cmd_stage_set_link_queue_in_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_link,
		(void *)&stage_dev,
		(void *)&stage_queue,
		(void *)&stage_in_queue,
		(void *)&stage_in_qid,
		NULL,
	},
};

static char const
cmd_stage_set_link_queue_out_help[] = "stage set <stage_name> link <name> queue out <qid> ";

cmdline_parse_inst_t stage_set_link_queue_out_cmd_ctx = {
	.f = cli_stage_set_link_queue_out,
	.data = NULL,
	.help_str = cmd_stage_set_link_queue_out_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_link,
		(void *)&stage_dev,
		(void *)&stage_queue,
		(void *)&stage_out_queue,
		(void *)&stage_out_qid,
		NULL,
	},
};

static char const
cmd_stage_set_graph_nodes_help[] = "stage set <stage_name> graph <node-list>";

cmdline_parse_inst_t stage_set_graph_nodes_cmd_ctx = {
	.f = cli_stage_set_graph_nodes,
	.data = NULL,
	.help_str = cmd_stage_set_graph_nodes_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_graph,
		(void *)&stage_nodes,
		NULL,
	},
};

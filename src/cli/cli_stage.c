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

	rte_strscpy(config.name, res->name, STAGE_NAME_MAX_LEN);
	config.name[strlen(res->name)] = '\0';
	config.coremask = res->mask;
	config.queue.type = STAGE_TYPE_MAX;

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
cli_stage_set_type_rx(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
	struct stage_queue_config queue_config;
        char stage_name[STAGE_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

	memset(&queue_config, 0, sizeof(queue_config));
	queue_config.type = STAGE_TYPE_RX;
	queue_config.rx.out = res->out_qid;

        rc = stage_config_set_queue(stage_name, &queue_config);
        if (rc < 0) {
                cmdline_printf(cl, "stage show %s failed: %s\n", stage_name, rte_strerror(-rc));
        }
}

static void
cli_stage_set_type_worker(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
	struct stage_queue_config queue_config;
        char stage_name[STAGE_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

	memset(&queue_config, 0, sizeof(queue_config));
	queue_config.type = STAGE_TYPE_WORKER;
	queue_config.worker.in = res->in_qid;
	queue_config.worker.out = res->out_qid;
	queue_config.worker.ev_queue_config.schedule_type =
		(strcmp(res->schedule_type, "atomic") == 0) ?
		RTE_SCHED_TYPE_ATOMIC :
		RTE_SCHED_TYPE_ORDERED;

        rc = stage_config_set_queue(stage_name, &queue_config);
        if (rc < 0) {
                cmdline_printf(cl, "stage set %s failed: %s\n", stage_name, rte_strerror(-rc));
        }
}

static void
cli_stage_set_type_tx(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
	struct stage_queue_config queue_config;
        char stage_name[STAGE_NAME_MAX_LEN];
	int rc = -ENOENT;

	rte_strscpy(stage_name, res->name, STAGE_NAME_MAX_LEN);
	stage_name[strlen(res->name)] = '\0';

	memset(&queue_config, 0, sizeof(queue_config));
	queue_config.type = STAGE_TYPE_TX;
	queue_config.tx.in = res->in_qid;
	queue_config.worker.ev_queue_config.schedule_type =
		(strcmp(res->schedule_type, "atomic") == 0) ?
		RTE_SCHED_TYPE_ATOMIC :
		RTE_SCHED_TYPE_ORDERED;

        rc = stage_config_set_queue(stage_name, &queue_config);
        if (rc < 0) {
                cmdline_printf(cl, "stage set %s failed: %s\n", stage_name, rte_strerror(-rc));
        }
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
cmdline_parse_token_string_t stage_stage_type =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, stage_type, "type");
cmdline_parse_token_string_t stage_type_rx =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, type, "rx");
cmdline_parse_token_string_t stage_type_worker =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, type, "worker");
cmdline_parse_token_string_t stage_type_tx =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, type, "tx");
cmdline_parse_token_string_t stage_in_queue =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, in_queue, "in_queue");
cmdline_parse_token_num_t stage_in_qid =
	TOKEN_NUM_INITIALIZER(struct stage_cmd_tokens, in_qid, RTE_UINT32);
cmdline_parse_token_string_t stage_out_queue =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, out_queue, "out_queue");
cmdline_parse_token_num_t stage_out_qid =
	TOKEN_NUM_INITIALIZER(struct stage_cmd_tokens, out_qid, RTE_UINT32);
cmdline_parse_token_string_t stage_schedule =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, schedule, "schedule");
cmdline_parse_token_string_t stage_schedule_type =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, schedule_type, "atomic#ordered");

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
cmd_stage_set_type_rx_help[] = "stage set <stage_name> type rx out_queue <out_qid>";

cmdline_parse_inst_t stage_set_type_rx_cmd_ctx = {
	.f = cli_stage_set_type_rx,
	.data = NULL,
	.help_str = cmd_stage_set_type_rx_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_stage_type,
		(void *)&stage_type_rx,
		(void *)&stage_out_queue,
		(void *)&stage_out_qid,
		NULL,
	},
};

static char const
cmd_stage_set_type_worker_help[] = "stage set <stage_name> type worker in_queue <in_qid> out_queue <out_qid> schedule <atomic#ordered>";

cmdline_parse_inst_t stage_set_type_worker_cmd_ctx = {
	.f = cli_stage_set_type_worker,
	.data = NULL,
	.help_str = cmd_stage_set_type_worker_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_stage_type,
		(void *)&stage_type_worker,
		(void *)&stage_in_queue,
		(void *)&stage_in_qid,
		(void *)&stage_out_queue,
		(void *)&stage_out_qid,
		(void *)&stage_schedule,
		(void *)&stage_schedule_type,
		NULL,
	},
};

static char const
cmd_stage_set_type_tx_help[] = "stage set <stage_name> type tx in_queue <in_qid> schedule <atomic#ordered>";

cmdline_parse_inst_t stage_set_type_tx_cmd_ctx = {
	.f = cli_stage_set_type_tx,
	.data = NULL,
	.help_str = cmd_stage_set_type_tx_help,
	.tokens = {
		(void *)&stage_cmd,
                (void *)&stage_set,
		(void *)&stage_name,
		(void *)&stage_stage_type,
		(void *)&stage_type_tx,
		(void *)&stage_in_queue,
		(void *)&stage_in_qid,
		(void *)&stage_schedule,
		(void *)&stage_schedule_type,
		NULL,
	},
};

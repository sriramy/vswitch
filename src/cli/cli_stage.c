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
#include "cli_stage.h"
#include "stage.h"

static char const
cmd_stage_add_help[] = "stage add <stage_name> [coremask <mask>]";

static char const
cmd_stage_rem_help[] = "stage rem <stage_name>";

static char const
cmd_stage_show_help[] = "stage show <stage_name>";

static void
cli_stage_add(void *parsed_result, struct cmdline *cl, __rte_unused void *data)
{
	struct stage_cmd_tokens *res = parsed_result;
        struct stage_config config;
	int rc;

	rte_strscpy(config.name, res->name, STAGE_NAME_MAX_LEN);
	config.name[strlen(res->name)] = '\0';
	config.coremask = res->mask;

	rc = stage_config_add(&config);
	if (rc < 0) {
                cmdline_printf(cl, "stage add %s failed: %s\n", config.name, rte_strerror(rte_errno));
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

cmdline_parse_token_string_t stage_cmd =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, stage, "stage");
cmdline_parse_token_string_t stage_add =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "add");
cmdline_parse_token_string_t stage_rem =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "rem");
cmdline_parse_token_string_t stage_show =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, action, "show");
cmdline_parse_token_string_t stage_name =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, name, NULL);
cmdline_parse_token_string_t stage_coremask =
	TOKEN_STRING_INITIALIZER(struct stage_cmd_tokens, coremask, "coremask");
cmdline_parse_token_num_t stage_mask =
	TOKEN_NUM_INITIALIZER(struct stage_cmd_tokens, mask, RTE_UINT32);

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

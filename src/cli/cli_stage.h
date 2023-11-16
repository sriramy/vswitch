/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_STAGE_H_
#define __VSWITCH_SRC_CLI_STAGE_H_

#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>

struct stage_config_cmd_tokens {
	cmdline_fixed_string_t stage;
	cmdline_fixed_string_t action;
	cmdline_fixed_string_t name;
};

extern cmdline_parse_inst_t stage_add_cmd_ctx;
extern cmdline_parse_inst_t stage_rem_cmd_ctx;
extern cmdline_parse_inst_t stage_show_cmd_ctx;

#endif /* __VSWITCH_SRC_CLI_STAGE_H_*/
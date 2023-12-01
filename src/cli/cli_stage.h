/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_STAGE_H_
#define __VSWITCH_SRC_CLI_STAGE_H_

#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>

struct stage_cmd_tokens {
	cmdline_fixed_string_t stage;
	cmdline_fixed_string_t action;
	cmdline_fixed_string_t name;
	cmdline_fixed_string_t coremask;
	cmdline_fixed_string_t type;
	cmdline_fixed_string_t stage_type;
	cmdline_fixed_string_t stage_queue;
	cmdline_fixed_string_t in_queue;
	cmdline_fixed_string_t out_queue;
	cmdline_fixed_string_t schedule;
	cmdline_fixed_string_t schedule_type;
	cmdline_fixed_string_t link;
	cmdline_fixed_string_t dev;
	uint32_t mask;
	uint8_t in_qid;
	uint8_t out_qid;
};

extern cmdline_parse_inst_t stage_add_cmd_ctx;
extern cmdline_parse_inst_t stage_rem_cmd_ctx;
extern cmdline_parse_inst_t stage_show_cmd_ctx;

extern cmdline_parse_inst_t stage_set_type_cmd_ctx;
extern cmdline_parse_inst_t stage_set_queue_in_cmd_ctx;
extern cmdline_parse_inst_t stage_set_queue_out_cmd_ctx;
extern cmdline_parse_inst_t stage_set_link_queue_cmd_ctx;

#endif /* __VSWITCH_SRC_CLI_STAGE_H_*/
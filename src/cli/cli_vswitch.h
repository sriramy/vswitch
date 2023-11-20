/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_VSWITCH_H_
#define __VSWITCH_SRC_CLI_VSWITCH_H_

#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>

struct vswitch_cmd_tokens {
	cmdline_fixed_string_t cmd;
	cmdline_fixed_string_t action;
};

extern cmdline_parse_inst_t vswitch_show_cmd_ctx;

#endif /* __VSWITCH_SRC_CLI_VSWITCH_H_*/
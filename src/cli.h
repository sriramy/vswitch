/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_H_
#define __VSWITCH_SRC_CLI_H_

#include <cmdline.h>
#include <cmdline_parse.h>

extern cmdline_parse_inst_t link_show_cmd_ctx;
extern cmdline_parse_inst_t link_dev_show_cmd_ctx;

extern cmdline_parse_ctx_t commands_ctx[];

#endif /* __VSWITCH_SRC_CLI_H_ */
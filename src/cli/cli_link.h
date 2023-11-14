/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_LINK_H_
#define __VSWITCH_SRC_CLI_LINK_H_

#include <cmdline.h>
#include <cmdline_parse.h>

struct link_show_cmd_tokens {
	cmdline_fixed_string_t cmd;
	cmdline_fixed_string_t dev;
	cmdline_fixed_string_t show;
};

struct link_config_cmd_tokens {
	cmdline_fixed_string_t cmd;
	cmdline_fixed_string_t dev;
	cmdline_fixed_string_t config;
	cmdline_fixed_string_t action;
	cmdline_fixed_string_t rxq;
	cmdline_fixed_string_t txq;
	cmdline_fixed_string_t mempool;
	cmdline_fixed_string_t mp_name;
	uint16_t nb_rxq;
	uint16_t nb_txq;
};

extern cmdline_parse_inst_t link_show_cmd_ctx;
extern cmdline_parse_inst_t link_dev_show_cmd_ctx;
extern cmdline_parse_inst_t link_dev_config_add_cmd_ctx;
extern cmdline_parse_inst_t link_dev_config_rem_cmd_ctx;
extern cmdline_parse_inst_t link_dev_config_show_cmd_ctx;

#endif /* __VSWITCH_SRC_CLI_LINK_H_ */
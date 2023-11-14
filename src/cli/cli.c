/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdint.h>
#include <linux/if.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_rdline.h>

#include "cli.h"
#include "cli_link.h"
#include "cli_mempool.h"

cmdline_parse_ctx_t commands_ctx[] = {
	(cmdline_parse_inst_t *)&link_show_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_show_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_add_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_rem_cmd_ctx,
	(cmdline_parse_inst_t *)&link_dev_config_show_cmd_ctx,

	(cmdline_parse_inst_t *)&mempool_add_cmd_ctx,
	(cmdline_parse_inst_t *)&mempool_rem_show_cmd_ctx,
	NULL,
};

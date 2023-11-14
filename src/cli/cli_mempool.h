/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_MEMPOOL_H_
#define __VSWITCH_SRC_CLI_MEMPOOL_H_

#include <rte_mempool.h>

#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>

struct mempool_config_cmd_tokens {
	cmdline_fixed_string_t mempool;
	cmdline_fixed_string_t action;
	cmdline_fixed_string_t name;
	cmdline_fixed_string_t size;
	cmdline_fixed_string_t mbufs;
	cmdline_fixed_string_t cache;
	cmdline_fixed_string_t numa;
	uint16_t mbuf_sz;
	uint16_t nb_mbufs;
	uint16_t cache_sz;
	uint16_t node;
};

extern cmdline_parse_inst_t mempool_add_cmd_ctx;
extern cmdline_parse_inst_t mempool_rem_show_cmd_ctx;

struct rte_mempool*
mempool_config_get_rte_mempool(char const *mp_name);

#endif /* __VSWITCH_SRC_CLI_MEMPOOL_H_*/
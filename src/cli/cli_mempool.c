/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <rte_malloc.h>
#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_num.h>

#include "cli.h"
#include "cli_mempool.h"
#include "mempool.h"

static char const
cmd_mempool_add_help[] = "mempool add <mp_name> [size <mbuf_sz>] [mbufs <nb_mbufs>] "
		     "[cache <cache_sz>] [numa <node>]";

static char const
cmd_mempool_rem_show_help[] = "mempool rem#show <mp_name>";


static void
cli_mempool_add(void *parsed_result, __rte_unused struct cmdline *cl, __rte_unused void *data)
{
	struct mempool_config_cmd_tokens *res = parsed_result;
        char mp_name[RTE_MEMPOOL_NAMESIZE];
        struct mempool_config config;
	int rc;

	rte_strscpy(mp_name, res->name, RTE_MEMPOOL_NAMESIZE);
	mp_name[strlen(res->name)] = '\0';

        strncpy(config.name, mp_name, strlen(mp_name));
	config.mbuf_sz = res->mbuf_sz;
	config.nb_mbufs = res->nb_mbufs;
	config.cache_sz = res->cache_sz;
	config.numa_node = res->node;

	rc = mempool_config_add(&config);
	if (rc < 0) {
                cmdline_printf(cl, "mempool add %s failed: %s\n", mp_name, rte_strerror(rte_errno));
        }
}

static void
cli_mempool_rem_show(void *parsed_result, __rte_unused struct cmdline *cl, __rte_unused void *data)
{
	struct mempool_config_cmd_tokens *res = parsed_result;
        char mp_name[RTE_MEMPOOL_NAMESIZE];
        struct mempool *mp;

	rte_strscpy(mp_name, res->name, RTE_MEMPOOL_NAMESIZE);
	mp_name[strlen(res->name)] = '\0';
        mp = mempool_config_get(mp_name);
        if (!mp) {
                cmdline_printf(cl, "%s\n", rte_strerror(ENOENT));
                goto err;
        }

        if (strcmp(res->action, "rem") == 0) {
                mempool_config_rem(mp_name);
        } else if (strcmp(res->action, "show") == 0) {
                cmdline_printf(cl,
                        "%s: nb_mbufs=%d, mbus_sz=%d, cache %d numa %d\n",
                        mp->config.name, mp->config.nb_mbufs, mp->config.mbuf_sz,
                        mp->config.cache_sz, mp->config.numa_node);
        } else {
                cmdline_printf(cl, MSG_ARG_INVALID, res->action);
        }

        return;

err:
        cmdline_printf(cl, MSG_CMD_FAIL, "mempool");
}

cmdline_parse_token_string_t mempool_cmd =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, mempool, "mempool");
cmdline_parse_token_string_t mempool_add =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, action, "add");
cmdline_parse_token_string_t mempool_rem_show =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, action, "rem#show");
cmdline_parse_token_string_t mempool_name =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, name, NULL);
cmdline_parse_token_string_t mempool_add_sz =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, size, "size");
cmdline_parse_token_num_t mempool_add_mbuf_sz =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, mbuf_sz, RTE_UINT16);
cmdline_parse_token_string_t mempool_add_mbufs =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, mbufs, "mbufs");
cmdline_parse_token_num_t mempool_add_nb_mbufs =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, nb_mbufs, RTE_UINT16);
cmdline_parse_token_string_t mempool_add_cache =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, cache, "cache");
cmdline_parse_token_num_t mempool_add_cache_sz =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, cache_sz, RTE_UINT16);
cmdline_parse_token_string_t mempool_add_numa =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, numa, "numa");
cmdline_parse_token_num_t mempool_add_node =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, node, RTE_UINT16);

cmdline_parse_inst_t mempool_add_cmd_ctx = {
	.f = cli_mempool_add,
	.data = NULL,
	.help_str = cmd_mempool_add_help,
	.tokens = {
		(void *)&mempool_cmd,
                (void *)&mempool_add,
		(void *)&mempool_name,
		(void *)&mempool_add_sz,
		(void *)&mempool_add_mbuf_sz,
		(void *)&mempool_add_mbufs,
		(void *)&mempool_add_nb_mbufs,
		(void *)&mempool_add_cache,
		(void *)&mempool_add_cache_sz,
		(void *)&mempool_add_numa,
		(void *)&mempool_add_node,
		NULL,
	},
};

cmdline_parse_inst_t mempool_rem_show_cmd_ctx = {
	.f = cli_mempool_rem_show,
	.data = NULL,
	.help_str = cmd_mempool_rem_show_help,
	.tokens = {
		(void *)&mempool_cmd,
                (void *)&mempool_rem_show,
		(void *)&mempool_name,
		NULL,
	},
};

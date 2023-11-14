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

static const char
cmd_mempool_add_help[] = "mempool add <mp_name> [size <mbuf_sz>] [mbufs <nb_mbufs>] "
		     "[cache <cache_sz>] [numa <node>]";

static const char
cmd_mempool_rem_show_help[] = "mempool rem#show <mp_name>";

static struct mempool_head mempool_node = TAILQ_HEAD_INITIALIZER(mempool_node);

static struct mempool*
mempool_config_get(char const *mp_name)
{
	struct mempool *mp;

	TAILQ_FOREACH(mp, &mempool_node, next) {
		if (strcmp(mp->config.name, mp_name) == 0)
			return mp;
	}
	return NULL;
}

static void
cli_mempool_config_add(void *parsed_result, __rte_unused struct cmdline *cl, __rte_unused void *data)
{
	struct mempool_config_cmd_tokens *res = parsed_result;
        char mp_name[RTE_MEMPOOL_NAMESIZE];
        struct mempool *mp;

	rte_strscpy(mp_name, res->name, RTE_MEMPOOL_NAMESIZE);
	mp_name[strlen(res->name)] = '\0';
        mp = mempool_config_get(mp_name);

	if (!mp) {
		mp = rte_malloc(NULL, sizeof(struct mempool), 0);
		if (!mp) {
                        cmdline_printf(cl, "%s\n", rte_strerror(rte_errno));
                        goto err;
                }
	} else {
                cmdline_printf(cl, MSG_ERR_EXIST);
		goto err;
        }

        strncpy(mp->config.name, mp_name, strlen(mp_name));
	mp->config.mbuf_sz = res->mbuf_sz;
	mp->config.nb_mbufs = res->nb_mbufs;
	mp->config.cache_sz = res->cache_sz;
	mp->config.numa_node = res->node;
	mp->mp = rte_pktmbuf_pool_create(
                mp->config.name,
                mp->config.nb_mbufs,
                mp->config.cache_sz,
		0,
                mp->config.mbuf_sz,
                mp->config.numa_node);
	if (!mp->mp) {
                rte_free(mp);
                cmdline_printf(cl, "%s: %s\n", mp->config.name, rte_strerror(rte_errno));
                goto err;
        }

	TAILQ_INSERT_TAIL(&mempool_node, mp, next);
	return;

err:
        cmdline_printf(cl, MSG_CMD_FAIL, "mempool");
}

static void
cli_mempool_config_rem_show(void *parsed_result, __rte_unused struct cmdline *cl, __rte_unused void *data)
{
	struct mempool_config_cmd_tokens *res = parsed_result;
        char mp_name[RTE_MEMPOOL_NAMESIZE];
        struct mempool *mp;

	rte_strscpy(mp_name, res->name, RTE_MEMPOOL_NAMESIZE);
	mp_name[strlen(res->name)] = '\0';
        mp = mempool_config_get(mp_name);
        if (!mp) {
                cmdline_printf(cl, MSG_ERR_NOENT);
                goto err;
        }

        if (strcmp(res->action, "rem") == 0) {
                TAILQ_REMOVE(&mempool_node, mp, next);
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

cmdline_parse_token_string_t mempool_config_mempool =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, mempool, "mempool");
cmdline_parse_token_string_t mempool_config_add =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, action, "add");
cmdline_parse_token_string_t mempool_config_rem_show =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, action, "rem#show");
cmdline_parse_token_string_t mempool_config_name =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, name, NULL);
cmdline_parse_token_string_t mempool_config_add_sz =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, size, "size");
cmdline_parse_token_num_t mempool_config_add_mbuf_sz =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, mbuf_sz, RTE_UINT16);
cmdline_parse_token_string_t mempool_config_add_mbufs =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, mbufs, "mbufs");
cmdline_parse_token_num_t mempool_config_add_nb_mbufs =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, nb_mbufs, RTE_UINT16);
cmdline_parse_token_string_t mempool_config_add_cache =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, cache, "cache");
cmdline_parse_token_num_t mempool_config_add_cache_sz =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, cache_sz, RTE_UINT16);
cmdline_parse_token_string_t mempool_config_add_numa =
	TOKEN_STRING_INITIALIZER(struct mempool_config_cmd_tokens, numa, "numa");
cmdline_parse_token_num_t mempool_config_add_node =
	TOKEN_NUM_INITIALIZER(struct mempool_config_cmd_tokens, node, RTE_UINT16);

cmdline_parse_inst_t mempool_config_add_cmd_ctx = {
	.f = cli_mempool_config_add,
	.data = NULL,
	.help_str = cmd_mempool_add_help,
	.tokens = {
		(void *)&mempool_config_mempool,
                (void *)&mempool_config_add,
		(void *)&mempool_config_name,
		(void *)&mempool_config_add_sz,
		(void *)&mempool_config_add_mbuf_sz,
		(void *)&mempool_config_add_mbufs,
		(void *)&mempool_config_add_nb_mbufs,
		(void *)&mempool_config_add_cache,
		(void *)&mempool_config_add_cache_sz,
		(void *)&mempool_config_add_numa,
		(void *)&mempool_config_add_node,
		NULL,
	},
};

cmdline_parse_inst_t mempool_config_rem_show_cmd_ctx = {
	.f = cli_mempool_config_rem_show,
	.data = NULL,
	.help_str = cmd_mempool_rem_show_help,
	.tokens = {
		(void *)&mempool_config_mempool,
                (void *)&mempool_config_rem_show,
		(void *)&mempool_config_name,
		NULL,
	},
};

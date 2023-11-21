/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include <rte_malloc.h>

#include "mempool.h"

static struct mempool_head mempool_node = TAILQ_HEAD_INITIALIZER(mempool_node);

struct mempool*
mempool_config_get(char const *name)
{
	struct mempool *mp;

	TAILQ_FOREACH(mp, &mempool_node, next) {
		if (strcmp(mp->config.name, name) == 0)
			return mp;
	}
	return NULL;
}

int
mempool_config_add(struct mempool_config *config)
{
	int rc = -EINVAL;
	struct mempool* mp;

	mp = mempool_config_get(config->name);
	if (!mp) {
		mp = rte_malloc(NULL, sizeof(struct mempool), 0);
		if (!mp) {
                        rc = -rte_errno;
                        goto err;
                }
	} else {
                return -EEXIST;
        }

	memcpy(&mp->config, config, sizeof(*config));
	mp->mp = rte_pktmbuf_pool_create(
                mp->config.name,
                mp->config.nb_mbufs,
                mp->config.cache_sz,
		0,
                mp->config.mbuf_sz,
                mp->config.numa_node);
	if (!mp->mp) {
                rc = -rte_errno;
                goto err;
        }

	TAILQ_INSERT_TAIL(&mempool_node, mp, next);
        return 0;

err:
        if (mp) {
                if (mp->mp)
                        rte_mempool_free(mp->mp);
                rte_free(mp);
        }
        return rc;
}

int
mempool_config_rem(char const *name)
{
        struct mempool *mp = mempool_config_get(name);
        if (mp) {
                TAILQ_REMOVE(&mempool_node, mp, next);
                if (mp->mp)
                        rte_mempool_free(mp->mp);
                rte_free(mp);
                return 0;
        }

        return -ENOENT;
}
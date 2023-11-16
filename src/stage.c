/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include <rte_malloc.h>
#include <rte_errno.h>

#include "stage.h"

struct stage *stage_array[STAGE_MAX];

static struct stage_head stage_node = TAILQ_HEAD_INITIALIZER(stage_node);

static int16_t 
stage_get_free_id() {
	int16_t i = 0;

	for (; i < STAGE_MAX; i++) {
		if (!stage_array[i]) {
			return i;
		}
	}

	return -1;
}

struct stage*
stage_config_get(char const *name)
{
	struct stage *s;

	TAILQ_FOREACH(s, &stage_node, next) {
		if (strcmp(s->config.name, name) == 0)
			return s;
	}
	return NULL;
}

int
stage_config_add(struct stage_config *config)
{
	int rc = -EINVAL;
	struct stage* s;
	struct lcore_head lcore_node = TAILQ_HEAD_INITIALIZER(lcore_node);

	s = stage_config_get(config->name);
	if (!s) {
		s = rte_malloc(NULL, sizeof(struct stage), 0);
		if (!s) {
                        rc = -rte_errno;
                        goto err;
                }
	} else {
                rc = -EEXIST;
		goto err;
        }

	memcpy(&s->config, config, sizeof(*config));
	s->config.stage_id = stage_get_free_id();
	if (s->config.stage_id < 0) {
		rc = -ENOMEM;
		goto err;
	}
	s->config.lcore_node = lcore_node;

	stage_array[s->config.stage_id] = s;
	TAILQ_INSERT_TAIL(&stage_node, s, next);
        return 0;

err:
	if (s)
		rte_free(s);
        return rc;
}

int
stage_config_rem(char const *name)
{
        struct stage *s = stage_config_get(name);
	struct lcore *l1, *l2;
        if (s) {
		l1 = TAILQ_FIRST(&s->config.lcore_node);
		while (l1 != NULL) {
			l2 = TAILQ_NEXT(l1, next);
			rte_free(l1);
			l1 = l2;
		}
		TAILQ_INIT(&s->config.lcore_node);

                TAILQ_REMOVE(&stage_node, s, next);
		stage_array[s->config.stage_id] = NULL;
		rte_free(s);
                return 0;
        }

        return -ENOENT;
}

struct lcore*
stage_config_lcore_get(char const *name, int lcore_id)
{
        struct stage *s = stage_config_get(name);
	struct lcore *l;

        if (s) {
		TAILQ_FOREACH(l, &s->config.lcore_node, next) {
			if (l->config.lcore_id == lcore_id)
				return l;
		}
	}

	return NULL;
}

int
stage_config_lcore_add(char const *name, struct lcore_config *config)
{
	int rc = -EINVAL;
	struct lcore* l;

        struct stage *s = stage_config_get(name);
        if (!s) {
		rc = -ENOENT;
		goto err;
	}

	l = stage_config_lcore_get(name, config->lcore_id);
	if (!l) {
		l = rte_malloc(NULL, sizeof(struct lcore), 0);
		if (!l) {
                        rc = -rte_errno;
                        goto err;
                }
	} else {
                rc = -EEXIST;
		goto err;
        }

	memcpy(&l->config, config, sizeof(*config));
	TAILQ_INSERT_TAIL(&s->config.lcore_node, l, next);
        return 0;

err:
        return rc;
}

int
stage_config_lcore_rem(char const *name, int lcore_id)
{
        struct stage *s = stage_config_get(name);
	struct lcore *l = stage_config_lcore_get(name, lcore_id);

        if (s && l) {
                TAILQ_REMOVE(&s->config.lcore_node, l, next);
		rte_free(l);
                return 0;
        }

        return -ENOENT;
}

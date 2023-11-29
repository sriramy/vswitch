/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include <rte_bitmap.h>
#include <rte_errno.h>
#include <rte_malloc.h>

#include "stage.h"

static void *enabled_cores_bitmap = NULL;
static struct rte_bitmap *enabled_cores = NULL;

static void *used_cores_bitmap = NULL;
static struct rte_bitmap *used_cores = NULL;

static struct stage *stage_array[STAGE_MAX];

static struct stage_head stage_node = TAILQ_HEAD_INITIALIZER(stage_node);

void
stage_init()
{
	uint16_t core_id;
	uint32_t nb_bytes;

	nb_bytes = rte_bitmap_get_memory_footprint(RTE_MAX_LCORE);
	if (!nb_bytes) {
		goto err;
	}

	enabled_cores_bitmap = rte_zmalloc("enabled-cores-bitmap",
		nb_bytes, RTE_CACHE_LINE_SIZE);
	if (!enabled_cores_bitmap) {
		goto err;
	}

	enabled_cores = rte_bitmap_init(RTE_MAX_LCORE,
		enabled_cores_bitmap, nb_bytes);
	if (!enabled_cores) {
		goto err;
	}

	used_cores_bitmap = rte_zmalloc("used-cores-bitmap",
		nb_bytes, RTE_CACHE_LINE_SIZE);
	if (!used_cores_bitmap) {
		goto err;
	}

	used_cores = rte_bitmap_init(RTE_MAX_LCORE,
		used_cores_bitmap, nb_bytes);
	if (!used_cores) {
		goto err;
	}

	RTE_LCORE_FOREACH_WORKER(core_id) {
		rte_bitmap_set(enabled_cores, core_id);
	}

	return;

err:
	if (enabled_cores_bitmap)
		rte_free(enabled_cores_bitmap);
	if (used_cores_bitmap)
		rte_free(used_cores_bitmap);
}

void
stage_quit()
{
	if (enabled_cores_bitmap)
		rte_free(enabled_cores_bitmap);
	if (used_cores_bitmap)
		rte_free(used_cores_bitmap);
}

uint64_t
stage_get_enabled_coremask()
{
	uint16_t core_id;
	uint64_t enabled_coremask = 0;
	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		if (rte_bitmap_get(enabled_cores, core_id))
			enabled_coremask |= (1UL << core_id);
	}

	return enabled_coremask;
}

uint64_t
stage_get_used_coremask()
{
	uint16_t core_id;
	uint64_t used_coremask = 0;
	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		if (rte_bitmap_get(used_cores, core_id))
			used_coremask |= (1UL << core_id);
	}

	return used_coremask;
}

static int
stage_get_free_id(uint32_t *stage_id) {
	int i = 0;

	for (i = 0; i < STAGE_MAX; i++) {
		if (!stage_array[i]) {
			*stage_id = i;
			break;
		}
	}

	return (i == STAGE_MAX) ? 0 : 1;
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
	uint16_t core_id;

	s = stage_config_get(config->name);
	if (!s) {
		s = rte_malloc(NULL, sizeof(struct stage), 0);
		if (!s) {
                        rc = -rte_errno;
                        goto err;
                }
	} else {
		return -EEXIST;
        }

	memcpy(&s->config, config, sizeof(*config));
	if (!stage_get_free_id(&s->config.stage_id)) {
		rc = -ENOMEM;
		goto err;
	}

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		if (s->config.coremask & (1UL << core_id)) {
			if (!rte_bitmap_get(enabled_cores, core_id)) {
				rc = -EINVAL;
				goto err;
			}
			if (rte_bitmap_get(used_cores, core_id)) {
				rc = -EEXIST;
				goto err;
			}
		}
	}

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		if (s->config.coremask & (1UL << core_id)) {
			rte_bitmap_set(used_cores, core_id);
		}
	}

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
	uint16_t core_id;

        if (s) {
		for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
			if (s->config.coremask & (1UL << core_id)) {
				rte_bitmap_clear(used_cores, core_id);
			}
		}

                TAILQ_REMOVE(&stage_node, s, next);
		stage_array[s->config.stage_id] = NULL;
		rte_free(s);
                return 0;
        }

        return -ENOENT;
}

int
stage_config_set_queue(char const *name, struct stage_queue_config *config)
{
        struct stage *s = stage_config_get(name);

        if (s) {
		s->config.queue = *config;
                return 0;
        }

        return -ENOENT;
}

int
stage_config_walk(stage_config_cb cb, void *data)
{
	struct stage *s;
	int rc = 0;

	TAILQ_FOREACH(s, &stage_node, next) {
		rc = cb(&s->config, data);
		if (rc < 0) {
			return rc;
		}
	}

	return rc;
}
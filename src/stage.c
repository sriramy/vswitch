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

static uint64_t enabled_coremask = 0;

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
		enabled_coremask |= (1UL << core_id);
		rte_bitmap_set(enabled_cores, core_id);
	}

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		rte_bitmap_clear(used_cores, core_id);
	}

	return;

err:
	if (enabled_cores_bitmap)
		rte_free(enabled_cores_bitmap);
	if (used_cores_bitmap)
		rte_free(used_cores_bitmap);
}

void
stage_uninit()
{
	if (enabled_cores_bitmap)
		rte_free(enabled_cores_bitmap);
	if (used_cores_bitmap)
		rte_free(used_cores_bitmap);
}

uint64_t
stage_get_enabled_coremask()
{
	return enabled_coremask;
}

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
	uint16_t mask, i;

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

	mask = s->config.coremask;
	for (i = 0; i < RTE_MAX_LCORE && mask; i++) {
		if (!(mask & 1UL)) {
			continue;
		}
		if (!rte_bitmap_get(enabled_cores, i)) {
			rc = -EINVAL;
			goto err;
		}
		if (rte_bitmap_get(used_cores, i)) {
			rc = -EEXIST;
			goto err;
		}
		mask = mask >> 1;
	}

	mask = s->config.coremask;
	for (i = 0; i < RTE_MAX_LCORE && mask; i++) {
		if (!(mask & 1UL)) {
			continue;
		}
		rte_bitmap_set(used_cores, i);
		mask = mask >> 1;
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
	uint16_t i;

        if (s) {
		for (i = 0; i < RTE_MAX_LCORE; i++) {
			if (!(s->config.coremask & (1UL << i))) {
				continue;
			}
			rte_bitmap_clear(used_cores, i);
		}

                TAILQ_REMOVE(&stage_node, s, next);
		stage_array[s->config.stage_id] = NULL;
		rte_free(s);
                return 0;
        }

        return -ENOENT;
}

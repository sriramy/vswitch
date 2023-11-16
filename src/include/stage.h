/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_STAGE_H_
#define __VSWITCH_SRC_API_STAGE_H_

#include <stdio.h>
#include <stdlib.h>

#include <rte_bitmap.h>

#define STAGE_NAME_MAX_LEN (64)
#define STAGE_MAX (16)

struct lcore_config {
	int lcore_id;
	int stage_id;
	int queue_id;
};

struct lcore {
	TAILQ_ENTRY(lcore) next;
	struct lcore_config config;
};

struct stage_config {
	char name[STAGE_NAME_MAX_LEN];
	int stage_id;
	TAILQ_HEAD(lcore_head, lcore) lcore_node;
};

struct stage {
	TAILQ_ENTRY(stage) next;
	struct stage_config config;
};
TAILQ_HEAD(stage_head, stage);

struct stage* stage_config_get(char const *name);
int stage_config_add(struct stage_config *config);
int stage_config_rem(char const *name);
struct lcore* stage_config_lcore_get(char const *name, int lcore_id);
int stage_config_lcore_add(char const *name, struct lcore_config *config);
int stage_config_lcore_rem(char const *name, int lcore_id);

#endif /* __VSWITCH_SRC_API_STAGE_H_ */
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

struct stage_config {
	char name[STAGE_NAME_MAX_LEN];
	int stage_id;
	uint32_t coremask;
};

struct stage {
	TAILQ_ENTRY(stage) next;
	struct stage_config config;
};
TAILQ_HEAD(stage_head, stage);

void stage_init();
void stage_uninit();

struct stage* stage_config_get(char const *name);
int stage_config_add(struct stage_config *config);
int stage_config_rem(char const *name);

#endif /* __VSWITCH_SRC_API_STAGE_H_ */
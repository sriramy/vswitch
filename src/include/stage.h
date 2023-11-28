/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_STAGE_H_
#define __VSWITCH_SRC_API_STAGE_H_

#define STAGE_NAME_MAX_LEN	(64)
#define STAGE_MAX		(16)
#define STAGE_QUEUE_MAX		(4)

struct stage_queue_info {

};

struct stage_config {
	char name[STAGE_NAME_MAX_LEN];
	uint32_t stage_id;
	uint32_t coremask;
	uint32_t nb_queues;
	struct stage_queue_info queue_info[STAGE_QUEUE_MAX];
};

struct stage {
	TAILQ_ENTRY(stage) next;
	struct stage_config config;
};
TAILQ_HEAD(stage_head, stage);

void stage_init();
void stage_uninit();

uint64_t stage_get_enabled_coremask();
uint64_t stage_get_used_coremask();

struct stage* stage_config_get(char const *name);
int stage_config_add(struct stage_config *config);
int stage_config_rem(char const *name);

#endif /* __VSWITCH_SRC_API_STAGE_H_ */
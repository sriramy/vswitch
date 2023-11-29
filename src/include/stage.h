/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_STAGE_H_
#define __VSWITCH_SRC_API_STAGE_H_

#define STAGE_NAME_MAX_LEN	(64)
#define STAGE_MAX		(16)

enum {
	STAGE_TYPE_RX = 0,
	STAGE_TYPE_WORKER,
	STAGE_TYPE_TX,
	STAGE_TYPE_MAX
};

struct stage_queue_config {
	uint8_t type;
	union {
		struct {
			uint8_t rsvd;
			uint8_t out;
		} rx;
		struct {
			uint8_t in;
			uint8_t out;
		} worker;
		struct {
			uint8_t in;
			uint8_t rsvd;
		} tx;
	};
	uint8_t rsvd1;
};

struct stage_config {
	char name[STAGE_NAME_MAX_LEN];
	uint32_t stage_id;
	uint32_t coremask;
	struct stage_queue_config queue;
};

struct stage {
	TAILQ_ENTRY(stage) next;
	struct stage_config config;
};
TAILQ_HEAD(stage_head, stage);

typedef int (*stage_config_cb) (struct stage_config *config, void *data);

void stage_init();
void stage_quit();

uint64_t stage_get_enabled_coremask();
uint64_t stage_get_used_coremask();

struct stage* stage_config_get(char const *name);
int stage_config_add(struct stage_config *config);
int stage_config_rem(char const *name);

int stage_config_set_queue(char const *name, struct stage_queue_config *config);
int stage_config_walk(stage_config_cb cb, void *data);

#endif /* __VSWITCH_SRC_API_STAGE_H_ */
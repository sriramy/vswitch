/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_STAGE_H_
#define __VSWITCH_SRC_API_STAGE_H_

#include <rte_eventdev.h>

#define STAGE_NAME_MAX_LEN	(64)
#define STAGE_MAX		(16)
#define STAGE_MAX_LINK_QUEUES	(8)

extern const char *stage_type_str[];

enum {
	STAGE_TYPE_RX = 0,
	STAGE_TYPE_WORKER,
	STAGE_TYPE_TX,
	STAGE_TYPE_MAX
};

struct stage_link_queue_config {
	uint8_t enabled;
	uint8_t link_id;
	uint8_t queue_id;
};

struct stage_ev_queue_config {
	uint8_t in;
	uint8_t sched_type_in;
	uint8_t out;
	uint8_t sched_type_out;
	struct rte_event_queue_conf config_in;
};

struct stage_config {
	char name[STAGE_NAME_MAX_LEN];
	uint32_t stage_id;
	uint32_t coremask;
	uint8_t type;
	struct stage_link_queue_config link_in_queue[STAGE_MAX_LINK_QUEUES];
	struct stage_link_queue_config link_out_queue[STAGE_MAX_LINK_QUEUES];
	struct stage_ev_queue_config ev_queue;
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

int stage_config_set_type(char const *name, uint8_t type);
int stage_config_set_ev_queue_in(char const *name, uint8_t qid, uint8_t schedule_type);
int stage_config_set_ev_queue_out(char const *name, uint8_t qid, uint8_t schedule_type);
int stage_config_set_link_queue_in(char const *name, char const *link_name, uint8_t qid);
int stage_config_set_link_queue_out(char const *name, char const *link_name, uint8_t qid);

int stage_config_walk(stage_config_cb cb, void *data);

#endif /* __VSWITCH_SRC_API_STAGE_H_ */
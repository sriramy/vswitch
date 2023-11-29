/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_VSWITCH_H_
#define __VSWITCH_SRC_API_VSWITCH_H_

#include <rte_eventdev.h>

struct vswitch_config {
	int eventdev_id;
	struct rte_event_dev_info eventdev_info;
};

int vswitch_init();
int vswitch_quit();
struct vswitch_config* vswitch_config_get();

int vswitch_start();

#endif /* __VSWITCH_SRC_API_VSWITCH_H_ */
/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_EVENTDEV_DISPATCHER_H__
#define __SRC_LIB_EVENTDEV_DISPATCHER_H__

#include <rte_graph.h>

int eventdev_dispatcher_set_next(const char* next_node, uint16_t port_id, uint16_t queue_id);

#endif /* __SRC_LIB_EVENTDEV_DISPATCHER_H__ */

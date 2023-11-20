/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_VSWITCH_H_
#define __VSWITCH_SRC_API_VSWITCH_H_

#define VSWITCH_RX_QUEUES_PER_LCORE_MAX	(16)

struct vswitch_rxq_conf {
	uint16_t port_id;
	uint8_t queue_id;
};

struct vswitch_lcore_conf {
	uint16_t nb_rxq;
	struct vswitch_rxq_conf rxq[VSWITCH_RX_QUEUES_PER_LCORE_MAX];
} __rte_cache_aligned;

int vswitch_start();

#endif /* __VSWITCH_SRC_API_VSWITCH_H_ */
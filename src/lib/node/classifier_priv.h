/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __SRC_LIB_NODE_CLASSIFIER_PRIV_H__
#define __SRC_LIB_NODE_CLASSIFIER_PRIV_H__

#include <rte_common.h>

#define OBJS_PER_CLINE (RTE_CACHE_LINE_SIZE / sizeof(void *))

enum classifier_next_nodes {
	CLASSIFIER_NEXT_KERNEL_TX = 0,
	CLASSIFIER_NEXT_IP4_LOOKUP,
	CLASSIFIER_NEXT_IP6_LOOKUP,
	CLASSIFIER_NEXT_PKT_DROP,
	CLASSIFIER_NEXT_MAX,
};

struct classifier_node_ctx {
	uint16_t l2l3_type;
};

#endif /* __SRC_LIB_NODE_CLASSIFIER_PRIV_H__ */

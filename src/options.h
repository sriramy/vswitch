/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_OPTIONS_H_
#define __VSWITCH_SRC_OPTIONS_H_

#include <stdlib.h>
#include <stdio.h>

struct params {
	char *config;
	bool enable_graph_stats;
};

static const struct params params_default = {
	.config = "/etc/vswitch/xcluster.cli",
	.enable_graph_stats = true,
};

int options_parse(struct params *p, int argc, char **argv);

#endif /* __VSWITCH_SRC_OPTIONS_H_ */
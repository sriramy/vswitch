/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdlib.h>
#include <stdio.h>

struct params {
	char *config;
	bool enable_graph_stats;
};

static const struct params default_params = {
	.config = "/etc/vswitch/init.cli",
	.enable_graph_stats = true,
};

int options_parse(struct params *p, int argc, char **argv);

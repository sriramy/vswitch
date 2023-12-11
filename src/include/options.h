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
	char *graph_stats;
	char *host;
	uint16_t port;
	bool enable_graph_stats;
	bool enable_graph_pcap;
};

static const struct params params_default = {
	.config = "/etc/vswitch/xcluster.cli",
	.graph_stats = NULL,
	.host = "0.0.0.0",
	.port = 8086,
	.enable_graph_stats = false,
	.enable_graph_pcap = false,
};

int options_parse(struct params *p, int argc, char **argv);

#endif /* __VSWITCH_SRC_OPTIONS_H_ */
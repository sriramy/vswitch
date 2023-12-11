/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "options.h"

static char const usage[] = 
	"%s EAL_ARGS --"
	" -f CONFIG_FILE"
	" -g GRAPH_STATS_FILE"
	" -H host"
	" -P port"
        " -G [--enable-graph-stats]"
	" -h [--help]\n";

static const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"enable-graph-stats", no_argument, NULL, 's'},
	{"enable-graph-pcap", no_argument, NULL, 'p'},
	{NULL, 0, NULL, 0}
};

int options_parse(struct params *p, int argc, char **argv)
{
	char *app_name = argv[0];
	char ch;

	while ((ch = getopt_long(argc, argv, "hf:g:", long_options, NULL)) != -1) {
		switch (ch)
		{
		case 'f':
			p->config = strdup(optarg);
			break;
		case 'g':
			p->graph_stats = strdup(optarg);
			break;
		case 'H':
			p->host = strdup(optarg);
			break;
		case 'P':
			p->port = (uint16_t)atoi(optarg);
			break;
		case 's':
			p->enable_graph_stats = true;
			break;
		case 'p':
			p->enable_graph_pcap = true;
			break;
		case 'h':
		default:
			printf(usage, app_name);
			return -1;
		}
	}

        return 0;
}

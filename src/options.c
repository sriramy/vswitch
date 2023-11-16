/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

static char const usage[] = 
	"%s EAL_ARGS --"
	" -f CONFIG_FILE"
        " --enable-graph-stats"
	" --help\n";

static const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"enable-graph-stats", no_argument, NULL, 'g'},
	{NULL, 0, NULL, 0}
};

int options_parse(struct params *p, int argc, char **argv)
{
	char *app_name = argv[0];
	char ch;

	while ((ch = getopt_long(argc, argv, "hf:", long_options, NULL)) != -1) {
		switch (ch)
		{
		case 'f':
			p->config = strdup(optarg);
			break;
		case 'g':
			p->enable_graph_stats = true;
			break;
		case 'h':
		default:
			printf(usage, app_name);
			return -1;
		}
	}

        return 0;
}

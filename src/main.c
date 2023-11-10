/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <rte_cycles.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_log.h>

#include "log.h"
#include "options.h"

static volatile int force_quit = 0;

static struct params p = default_params;

static void
signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		force_quit = true;
		rte_wmb();
	}
}

int main(int argc, char **argv)
{
	int ret;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

	ret = options_parse(&p, argc, argv);
	if (ret < 0) {
	        RTE_LOG(CRIT, USER1, "invalid %s arguments\n", argv[0]);
		goto error;
	}

	rte_delay_ms(1);

	/* Dispatch loop */
	while (!force_quit) {
		/* Add cli handling here */
		rte_delay_ms(100);
	}

	rte_eal_cleanup();
	return 0;

error:
	return -1;
}

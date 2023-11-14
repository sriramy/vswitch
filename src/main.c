/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <rte_cycles.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_log.h>
#include <cmdline.h>
#include <cmdline_socket.h>

#include "cli/cli.h"
#include "log.h"
#include "options.h"

static volatile int force_quit = 0;

static struct params p = params_default;

#define MAX_BACKTRACE   32
static void signal_handler(int sig)
{
        char const *signal_str = sigabbrev_np(sig);

	if (sig == SIGINT) {
		signal(sig, SIG_IGN);
		printf("Ctrl-C> Do you really want to quit? [y/n] ");
		fflush(stdout);
		char c = getchar();
		if (c == 'y' || c == 'Y') {
			force_quit = true;
			return;
		}

		signal(sig, signal_handler);
		return;
	}

	RTE_LOG(CRIT, USER1, "Caught %s\n\n", signal_str);

	void *array[MAX_BACKTRACE];
	size_t size = backtrace(array, MAX_BACKTRACE);
	char **strings = backtrace_symbols (array, size);
	RTE_LOG(CRIT, USER1, "Obtained %zd stack frames.\n", size);
	for (size_t i = 0; i < size; i++) {
		RTE_LOG(CRIT, USER1, "%s\n", strings[i]);
	}

	free (strings);
	abort();
}

int main(int argc, char **argv)
{
	int ret;
	struct cmdline *cl;

	signal(SIGSEGV, signal_handler);
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

	cl = cmdline_stdin_new(commands_ctx, "graph> ");
	if (cl == NULL)
		return -1;

	rte_delay_ms(1);

	/* Dispatch loop */
	while (!force_quit) {
		cmdline_interact(cl);
	}

	cmdline_stdin_exit(cl);
	rte_eal_cleanup();
	return 0;

error:
	return -1;
}

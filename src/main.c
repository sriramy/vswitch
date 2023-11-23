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
#include <rte_errno.h>
#include <rte_log.h>
#include <cmdline.h>
#include <cmdline_socket.h>

#include "cli/cli.h"
#include "conn.h"
#include "log.h"
#include "options.h"
#include "stage.h"

static char const prompt[] = "vswitch> ";
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
			cli_quit();
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

static uint32_t
conn_thread(void *arg)
{
	struct params *vswitch_params = (struct params *)arg;
	int ret;

	struct conn_config config = {
		.welcome = "Welcome to vswitch 0.1.0\n",
		.prompt = prompt,
		.addr = strdup(vswitch_params->host),
		.port = vswitch_params->port,
	};

	ret = conn_init(&config);
	if (ret < 0) {
		RTE_LOG(CRIT, USER1, "conn_init failed (%s)\n",
			rte_strerror(-ret));
		goto err;
	}

	while(1) {
		conn_interact();
		rte_delay_ms(100);
	}

	conn_free();

err:
	return ret;
}

int main(int argc, char **argv)
{
	int ret;
	rte_thread_t conn_tid;

	struct sigaction action = {
		.sa_handler = signal_handler,
	};
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGSEGV, &action, NULL);

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

	ret = cli_init(prompt);
	if (ret < 0) {
		RTE_LOG(CRIT, USER1, "cli_init failed\n");
		goto error;
	}

	stage_init();

	rte_delay_ms(1);

	ret = cli_execute(p.config);
	if (ret < 0)
		RTE_LOG(CRIT, USER1, "cli_execute failed (%s)\n",
			rte_strerror(-ret));

	ret = rte_thread_create_control(&conn_tid, "conn-interact", conn_thread, &p);
        if (ret < 0) {
		RTE_LOG(CRIT, USER1, "Cannot create conn-interact thread\n");
		goto error;
	}

	cli_interact();

	rte_thread_join(conn_tid, NULL);
	rte_eal_mp_wait_lcore();

error:
	stage_uninit();
	cli_quit();
	rte_eal_cleanup();

	return ret;
}

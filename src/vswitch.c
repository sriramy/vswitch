/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_eventdev.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>

#include "link.h"
#include "stage.h"
#include "vswitch.h"

#define DEFAULT_PKT_BURST (32)

static struct vswitch_config *config = NULL;

int
fwd_pkts(uint16_t link_id, uint16_t peer_link_id)
{
	struct rte_mbuf *mbufs[DEFAULT_PKT_BURST];
	uint16_t rc;

	rc = rte_eth_rx_burst(link_id, 0, mbufs, DEFAULT_PKT_BURST);
	if (rc > 0) {
		rte_eth_tx_burst(peer_link_id, 0, mbufs, rc);
	}

	return rc;
}

static int launch_worker(__attribute__((unused)) void *arg) {
	uint16_t lcore_id = rte_lcore_id();

	RTE_LOG(INFO, USER1, "Worker %u starting\n", lcore_id);

	while(1) {
		link_map_walk(fwd_pkts);
	}

	RTE_LOG(INFO, USER1, "Worker %u stopping\n", lcore_id);

	return 0;
}

int
vswitch_init()
{
	int num_eventdev = 0;
	int rc = -EINVAL;

	if (!config) {
		config = rte_malloc(NULL, sizeof(struct vswitch_config), 0);
		if (!config) {
			rc = -rte_errno;
			goto err;
		}
	}

	num_eventdev = rte_event_dev_count();
	if (num_eventdev < 1) {
		rc = -ENOENT;
		goto err;
	}

	// TODO: Pick eventdev based on configuration instead
	memset(config, 0, sizeof(*config));
	config->eventdev_id = 0;
	rc = rte_event_dev_info_get(config->eventdev_id, &config->eventdev_info);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	return 0;

err:
	return rc;
}

int
vswitch_quit()
{
	rte_free(config);
	return 0;
}

struct vswitch_config *
vswitch_config_get()
{
	return config;
}

static int stage_configure(__rte_unused struct stage_config *stage_config, __rte_unused void *data) {
	config->nb_ports++;
	return 0;
}

int
vswitch_start()
{
	uint16_t lcore_id;

	link_start();
	stage_config_walk(stage_configure, config);

	RTE_LCORE_FOREACH_WORKER(lcore_id) {
		rte_eal_remote_launch(launch_worker, NULL, lcore_id);
	}

	return 0;
}
/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include "link.h"
#include "vswitch.h"

#define DEFAULT_PKT_BURST (32)

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
vswitch_start()
{
	uint16_t lcore_id;

	RTE_LCORE_FOREACH_WORKER(lcore_id) {
		rte_eal_remote_launch(launch_worker, NULL, lcore_id);
	}

	return 0;
}
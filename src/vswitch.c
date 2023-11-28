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

// static int
// vswitch_stage_setup()
// {
// 	const uint8_t dev_id = 0;
// 	/* +1 stages is for a SINGLE_LINK TX stage */
// 	const uint8_t nb_queues = cdata.num_stages + 1;
// 	const uint8_t nb_ports = cdata.num_workers;
// 	struct rte_event_dev_config config = {
// 		.nb_event_queues = nb_queues,
// 		.nb_event_ports = nb_ports,
// 		.nb_single_link_event_port_queues = 1,
// 		.nb_events_limit  = 4096,
// 		.nb_event_queue_flows = 1024,
// 		.nb_event_port_dequeue_depth = 128,
// 		.nb_event_port_enqueue_depth = 128,
// 	};
// 	struct rte_event_port_conf wkr_p_conf = {
// 		.dequeue_depth = cdata.worker_cq_depth,
// 		.enqueue_depth = 64,
// 		.new_event_threshold = 4096,
// 		.event_port_cfg = RTE_EVENT_PORT_CFG_HINT_WORKER,
// 	};
// 	struct rte_event_queue_conf wkr_q_conf = {
// 		.schedule_type = cdata.queue_type,
// 		.priority = RTE_EVENT_DEV_PRIORITY_NORMAL,
// 		.nb_atomic_flows = 1024,
// 		.nb_atomic_order_sequences = 1024,
// 	};
// 	struct rte_event_queue_conf tx_q_conf = {
// 		.priority = RTE_EVENT_DEV_PRIORITY_HIGHEST,
// 		.event_queue_cfg = RTE_EVENT_QUEUE_CFG_SINGLE_LINK,
// 	};
// 	struct port_link worker_queues[MAX_NUM_STAGES];
// 	uint8_t disable_implicit_release;
// 	unsigned int i;
// 	int ret, ndev = rte_event_dev_count();
// 	if (ndev < 1) {
// 	printf("%d: No Eventdev Devices Found\n", __LINE__);
// 	return -1;
// 	}
// 	struct rte_event_dev_info dev_info;
// 	ret = rte_event_dev_info_get(dev_id, &dev_info);
// 	printf("\tEventdev %d: %s\n", dev_id, dev_info.driver_name);
// 	disable_implicit_release = (dev_info.event_dev_cap &
// 		RTE_EVENT_DEV_CAP_IMPLICIT_RELEASE_DISABLE);
// 	wkr_p_conf.event_port_cfg = disable_implicit_release ?
// 	RTE_EVENT_PORT_CFG_DISABLE_IMPL_REL : 0;
// 	if (dev_info.max_num_events < config.nb_events_limit)
// 	config.nb_events_limit = dev_info.max_num_events;
// 	if (dev_info.max_event_port_dequeue_depth <
// 		config.nb_event_port_dequeue_depth)
// 	config.nb_event_port_dequeue_depth =
// 		dev_info.max_event_port_dequeue_depth;
// 	if (dev_info.max_event_port_enqueue_depth <
// 		config.nb_event_port_enqueue_depth)
// 	config.nb_event_port_enqueue_depth =
// 		dev_info.max_event_port_enqueue_depth;
// 	ret = rte_event_dev_configure(dev_id, &config);
// 	if (ret < 0) {
// 	printf("%d: Error configuring device\n", __LINE__);
// 	return -1;
// 	}
// 	/* Q creation - one load balanced per pipeline stage*/
// 	printf("  Stages:\n");
// 	for (i = 0; i < cdata.num_stages; i++) {
// 	if (rte_event_queue_setup(dev_id, i, &wkr_q_conf) < 0) {
// 		printf("%d: error creating qid %d\n", __LINE__, i);
// 		return -1;
// 	}
// 	cdata.qid[i] = i;
// 	cdata.next_qid[i] = i+1;
// 	worker_queues[i].queue_id = i;
// 	if (cdata.enable_queue_priorities) {
// 		/* calculate priority stepping for each stage, leaving
// 		* headroom of 1 for the SINGLE_LINK TX below
// 		*/
// 		const uint32_t prio_delta =
// 		(RTE_EVENT_DEV_PRIORITY_LOWEST-1) /  nb_queues;
// 		/* higher priority for queues closer to tx */
// 		wkr_q_conf.priority =
// 		RTE_EVENT_DEV_PRIORITY_LOWEST - prio_delta * i;
// 	}
// 	const char *type_str = "Atomic";
// 	switch (wkr_q_conf.schedule_type) {
// 	case RTE_SCHED_TYPE_ORDERED:
// 		type_str = "Ordered";
// 		break;
// 	case RTE_SCHED_TYPE_PARALLEL:
// 		type_str = "Parallel";
// 		break;
// 	}
// 	printf("\tStage %d, Type %s\tPriority = %d\n", i, type_str,
// 		wkr_q_conf.priority);
// 	}
// 	printf("\n");
// 	/* final queue for sending to TX core */
// 	if (rte_event_queue_setup(dev_id, i, &tx_q_conf) < 0) {
// 	printf("%d: error creating qid %d\n", __LINE__, i);
// 	return -1;
// 	}
// 	cdata.tx_queue_id = i;
// 	if (wkr_p_conf.new_event_threshold > config.nb_events_limit)
// 		wkr_p_conf.new_event_threshold = config.nb_events_limit;
// 	if (wkr_p_conf.dequeue_depth > config.nb_event_port_dequeue_depth)
// 		wkr_p_conf.dequeue_depth = config.nb_event_port_dequeue_depth;
// 	if (wkr_p_conf.enqueue_depth > config.nb_event_port_enqueue_depth)
// 		wkr_p_conf.enqueue_depth = config.nb_event_port_enqueue_depth;
// 	/* set up one port per worker, linking to all stage queues */
// 	for (i = 0; i < cdata.num_workers; i++) {
// 		struct worker_data *w = &worker_data[i];
// 		w->dev_id = dev_id;
// 		if (rte_event_port_setup(dev_id, i, &wkr_p_conf) < 0) {
// 			printf("Error setting up port %d\n", i);
// 			return -1;
// 		}
// 		uint32_t s;
// 		for (s = 0; s < cdata.num_stages; s++) {
// 			if (rte_event_port_link(dev_id, i,
// 			&worker_queues[s].queue_id,
// 			&worker_queues[s].priority,
// 			1) != 1) {
// 				printf("%d: error creating link for port %d\n",
// 					__LINE__, i);
// 				return -1;
// 			}
// 		}
// 		w->port_id = i;
// 	}
// 	ret = rte_event_dev_service_id_get(dev_id,
// 		&fdata->evdev_service_id);
// 	if (ret != -ESRCH && ret != 0) {
// 	printf("Error getting the service ID for sw eventdev\n");
// 	return -1;
// 	}
// 	rte_service_runstate_set(fdata->evdev_service_id, 1);
// 	rte_service_set_runstate_mapped_check(fdata->evdev_service_id, 0);
// 	return dev_id;
// }

int
vswitch_start()
{
	uint16_t lcore_id;

	RTE_LCORE_FOREACH_WORKER(lcore_id) {
		rte_eal_remote_launch(launch_worker, NULL, lcore_id);
	}

	return 0;
}
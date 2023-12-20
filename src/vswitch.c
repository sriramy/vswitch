/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_eventdev.h>
#include <rte_graph.h>
#include <rte_graph_worker.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_node_eth_api.h>
#include <rte_service.h>

#include "lcore.h"
#include "link.h"
#include "stage.h"
#include "vswitch.h"
#include "node/eventdev_dispatcher.h"
#include "node/eventdev_rx.h"
#include "node/eventdev_tx.h"
#include "node/forward.h"

#define DEFAULT_PKT_BURST (32)
#define DEFAULT_EVENT_BURST (32)

static struct vswitch_config *config = NULL;

int
vswitch_init(struct params *p)
{
	int num_eventdev = 0;
	uint16_t core_id;
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

	memset(config, 0, sizeof(*config));
	config->params = *p;
	config->ev_id = 0; // TODO: Pick event device based on configuration instead
	rc = rte_event_dev_info_get(config->ev_id, &config->ev_info);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	for (core_id = 0; core_id < RTE_MAX_LCORE; core_id++) {
		lcore_init(core_id, config->ev_id, &config->lcores[core_id]);
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

static int
stage_get_lcore_config(struct stage_config *stage_config, __rte_unused void *data)
{
	uint16_t core_id;

	RTE_LCORE_FOREACH_WORKER(core_id) {
		if (stage_config->coremask & (1UL << core_id)) {
			lcore_config_populate(stage_config, config->nb_ports++, &config->lcores[core_id]);
		}
	}

	if (stage_config->type == STAGE_TYPE_WORKER ||
	    stage_config->type == STAGE_TYPE_TX)
		config->nb_queues++;

	return 0;
}

static int
stage_configure_input_queues(__rte_unused struct stage_config *stage_config, __rte_unused void *data)
{
	int rc = 0;

	if (config->ev_info.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
		stage_config->ev_queue.config_in.event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;

	if (stage_config->type == STAGE_TYPE_WORKER ||
	    stage_config->type == STAGE_TYPE_TX) {
		rc = rte_event_queue_setup(config->ev_id,
					   stage_config->ev_queue.in,
					   &stage_config->ev_queue.config_in);
	}

	return rc;
}

int
vswitch_start()
{
	struct rte_event_dev_config ev_config;
	struct lcore_params *lcore;
	uint16_t core_id;
	int rc = -EINVAL;

	// Start all links
	link_start();

	stage_config_walk(stage_get_lcore_config, config);

	memset(&ev_config, 0, sizeof(ev_config));
	ev_config.nb_event_queues = config->nb_queues;
	ev_config.nb_event_ports = config->nb_ports;
	ev_config.nb_events_limit  = config->ev_info.max_num_events;
	ev_config.nb_event_queue_flows = 1024;
	ev_config.nb_event_port_dequeue_depth = config->ev_info.max_event_port_dequeue_depth;
	ev_config.nb_event_port_enqueue_depth = config->ev_info.max_event_port_enqueue_depth;
	rc = rte_event_dev_configure(config->ev_id, &ev_config);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	stage_config_walk(stage_configure_input_queues, config);

	RTE_LCORE_FOREACH_WORKER(core_id) {
		rc = lcore_graph_populate(&config->lcores[core_id], config->params.enable_graph_pcap);
		if (rc < 0)
			goto err;
	}

	rc = rte_event_dev_service_id_get(config->ev_id, &config->ev_service_id);
	if (rc != -ESRCH && rc != 0) {
		goto err;
	}
	rte_service_runstate_set(config->ev_service_id, 1);
	rte_service_set_runstate_mapped_check(config->ev_service_id, 0);

	rc = rte_event_dev_start(config->ev_id);
	if (rc < 0) {
		rc = -rte_errno;
		goto err;
	}

	RTE_LCORE_FOREACH_WORKER(core_id) {
		lcore = &config->lcores[core_id];

		lcore_graph_init(lcore);
		rte_eal_remote_launch(lcore_graph_worker, lcore, core_id);
	}

	return 0;

err:
	return rc;
}

int
vswitch_dump_stats(char const *file)
{
	const char *graph_disabled = "Graph stats not enabled";
	struct rte_graph_cluster_stats_param s_param;
	struct rte_graph_cluster_stats *stats;
	const char *pattern = "worker_*";
	FILE *fp = NULL;
	int rc = 0;

	/* Prepare stats object */
	fp = fopen(file, "w");
	if (fp == NULL) {
		rc = -rte_errno;
		goto err;
	}

	if (config->params.enable_graph_stats) {
		memset(&s_param, 0, sizeof(s_param));
		s_param.f = fp;
		s_param.socket_id = SOCKET_ID_ANY;
		s_param.graph_patterns = &pattern;
		s_param.nb_graph_patterns = 1;

		stats = rte_graph_cluster_stats_create(&s_param);
		if (stats == NULL) {
			rc = -EIO;
			goto err;
		}

		rte_graph_cluster_stats_get(stats, 0);
		rte_delay_ms(1E3);
	} else {
		fprintf(fp, "%s\n", graph_disabled);
	}

err:
	if (stats)
		rte_graph_cluster_stats_destroy(stats);
	if (fp)
		fclose(fp);
	return rc;
}

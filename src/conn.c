/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmdline.h>
#include <cmdline_parse_string.h>

#include <rte_malloc.h>
#include <rte_string_fns.h>

#include "conn.h"
#include "cli/cli.h"

static struct conn *conn = NULL;

static int control_cb(struct client_conn *client_conn);

static int
data_cb(struct client_conn *client_conn)
{
	char delim[] = "\n";
	char *line;
	int len;

	/* Read input message */
	memset(client_conn->buf, 0, sizeof(client_conn->buf));
	len = read(client_conn->fd, client_conn->buf, (CONN_BUF_LEN_MAX-1));
	if (len == -1) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
			return 0;
		}
		return -1;
	}

	if (len == 0)
		return 0;

	line = strtok(client_conn->buf, delim);
	while (line != NULL) {
		if (cmdline_in(client_conn->cl, line, strlen(line)) < 0)
			break;

		line = strtok(NULL, delim);
	}

	return 0;
}

static int
control_cb(struct client_conn *client_conn)
{
	int rc;

	rc = epoll_ctl(conn->fd_client_group, EPOLL_CTL_DEL, client_conn->fd, NULL);
	if (rc == -1)
		goto err;

	rc = close(client_conn->fd);
	if (rc == -1)
		goto err;

	cmdline_quit(client_conn->cl);

	rc = 0;

err:
	rte_free(client_conn);
	return rc;
}

int
conn_init(struct conn_config *config)
{
	int fd_client_group, fd_server, rc;
	struct sockaddr_in server_address;
	int reuse = 1;

	memset(&server_address, 0, sizeof(server_address));

	if ((config == NULL) || (config->welcome == NULL) ||
            (config->prompt == NULL) || (config->addr == NULL))
		goto err;

	rc = inet_aton(config->addr, &server_address.sin_addr);
	if (rc == 0)
		goto err;

	conn = rte_calloc("conn", 1, sizeof(struct conn), 0);
	if (conn == NULL)
		goto err;

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(config->port);

	fd_server = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (fd_server == -1) {
		goto err;
	}

	if (setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR,
                       (char const *)&reuse, sizeof(reuse)) < 0)
		goto err;

	rc = bind(fd_server, (struct sockaddr *)&server_address, sizeof(server_address));
	if (rc == -1)
		goto err;

	rc = listen(fd_server, SOMAXCONN);
	if (rc == -1)
		goto err;

	fd_client_group = epoll_create(1);
	if (fd_client_group == -1)
		goto err;

	rte_strscpy(conn->welcome, config->welcome, CONN_WELCOME_LEN_MAX);
	rte_strscpy(conn->prompt, config->prompt, CONN_PROMPT_LEN_MAX);
	conn->fd_server = fd_server;
	conn->fd_client_group = fd_client_group;

        return 0;

err:
	close(fd_server);
	conn_free(conn);
	conn = NULL;
	return -1;
}

void
conn_free()
{
	if (conn == NULL)
		return;

	if (conn->fd_client_group)
		close(conn->fd_client_group);

	if (conn->fd_server)
		close(conn->fd_server);

	rte_free(conn);
}

int
conn_accept()
{
	struct sockaddr_in client_address;
	socklen_t client_address_length;
	struct epoll_event event;
	int fd_client, rc;
	struct client_conn *client_conn;

	client_address_length = sizeof(client_address);
	fd_client = accept4(conn->fd_server, (struct sockaddr *)&client_address,
			    &client_address_length, SOCK_NONBLOCK);
	if (fd_client == -1) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
			return 0;

		return -1;
	}

	client_conn = rte_malloc("client-conn", sizeof(struct client_conn), 0);
	client_conn->fd = fd_client;
	client_conn->cl = cmdline_new(commands_ctx, conn->prompt, fd_client, fd_client);
	memset(client_conn->buf, 0, sizeof(client_conn->buf));

	event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	event.data.ptr = (void *)client_conn;
	rc = epoll_ctl(conn->fd_client_group, EPOLL_CTL_ADD, fd_client, &event);
	if (rc == -1) {
		goto err;
	}

	/* Client */
	rc = write(fd_client, conn->welcome, strlen(conn->welcome));
	if (rc == -1) {
		goto err;
	}

	return 0;

err:
	close(fd_client);
	return -1;
}

int
conn_poll()
{
	struct epoll_event event;
	struct client_conn *client_conn;
	int rc, rc_event = 0;

	rc = epoll_wait(conn->fd_client_group, &event, 1, 0);
	if ((rc == -1) || rc == 0)
		return rc;

	client_conn = (struct client_conn *)event.data.ptr;

	if (event.events & EPOLLIN)
		rc_event |= data_cb(client_conn);

	if (event.events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP))
		rc_event |= control_cb(client_conn);

	if (rc_event)
		return -1;

	return 0;
}

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

int
conn_init(struct conn_config *config)
{
	int fd_server, rc;
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
                       (const char *)&reuse, sizeof(reuse)) < 0)
		goto err;

	rc = bind(fd_server, (struct sockaddr *)&server_address, sizeof(server_address));
	if (rc == -1)
		goto err;

	rc = listen(fd_server, 1);
	if (rc == -1)
		goto err;

	/* Fill in */
	rte_strscpy(conn->welcome, config->welcome, CONN_WELCOME_LEN_MAX);
	rte_strscpy(conn->prompt, config->prompt, CONN_PROMPT_LEN_MAX);
	conn->fd_server = fd_server;

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

	if (conn->fd_server)
		close(conn->fd_server);

	rte_free(conn);
}

int
conn_interact()
{
	struct sockaddr_in client_address;
	socklen_t client_address_length;
	char buf[CONN_BUF_LEN_MAX];
	char delim[] = "\r\n";
	int fd_client, rc, len;
	struct cmdline *cl;
	char *line;

	/* Check input arguments */
	if (conn == NULL)
		return -1;

	/* Server socket */
	client_address_length = sizeof(client_address);
	fd_client = accept4(conn->fd_server, (struct sockaddr *)&client_address,
			    &client_address_length, SOCK_NONBLOCK);
	if (fd_client == -1) {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
			return 0;

		return -1;
	}

	/* Client */
	rc = write(fd_client, conn->welcome, strlen(conn->welcome));
	if (rc == -1) {
		close(fd_client);
		return -1;
	}

        cl = cmdline_new(commands_ctx, conn->prompt, -1, fd_client);
	while (1) {
		/* Read input message */
		memset(buf, 0, sizeof(buf));
		len = read(fd_client, buf, (CONN_BUF_LEN_MAX-1));
		if (len == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				rte_delay_ms(100);
				continue;
			}
			break;
		}

		if (len == 0)
			break;

		line = strtok(buf, delim);
		while (line != NULL) {
			if (cmdline_in(cl, line, strlen(line)) < 0)
				break;
			line = strtok(NULL, delim);
		}
	}

	cmdline_quit(cl);

	close(fd_client);
	return 0;
}
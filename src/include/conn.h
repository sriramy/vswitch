/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_API_CONN_H_
#define __VSWITCH_SRC_API_CONN_H_

#define CONN_WELCOME_LEN_MAX    (256)
#define CONN_PROMPT_LEN_MAX     (64)
#define CONN_BUF_LEN_MAX        (1024)

struct conn {
	char welcome[CONN_WELCOME_LEN_MAX];
	char prompt[CONN_PROMPT_LEN_MAX];
	int fd_server;
};

struct conn_config {
	const char *welcome;
	const char *prompt;
	const char *addr;
	uint16_t port;
};

int conn_init(struct conn_config *config);
void conn_free();
int conn_interact();

#endif /* __VSWITCH_SRC_API_CONN_H_ */
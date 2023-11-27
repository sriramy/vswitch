/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libmnl/libmnl.h>

#define NETLINK_SERVER_PID (4000)

typedef ssize_t (*orig_sendmsg)(int sockfd, const struct msghdr *msg, int flags);
ssize_t find_sendmsg(int sockfd, const struct msghdr *msg, int flags);
static orig_sendmsg original_sendmsg = find_sendmsg;

ssize_t find_sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	orig_sendmsg tmp = dlsym(RTLD_NEXT, "sendmsg");
	if (tmp == NULL) {
		perror("Could not find sendmsg");
		return -1;
	}

	original_sendmsg = tmp;
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	struct sockaddr_nl *nladdr = (struct sockaddr_nl *)msg->msg_name;
	if (nladdr && nladdr->nl_family == AF_NETLINK) {
		printf("Intercepted sendmsg for AF_NETLINK!\n");
	}
	return original_sendmsg(sockfd, msg, flags);
}

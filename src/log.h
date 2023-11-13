/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_LOG_H_
#define __VSWITCH_SRC_LOG_H_

#include <stdlib.h>
#include <stdio.h>

#define die(msg...) \
	do {                    \
		fprintf(stderr, msg); \
		exit(1); \
	} while(0);

#endif /* __VSWITCH_SRC_LOG_H_ */
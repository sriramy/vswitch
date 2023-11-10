/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#include <stdlib.h>
#include <stdio.h>

#define die(msg...) \
	do {                    \
		fprintf(stderr, msg); \
		exit(1); \
	} while(0);

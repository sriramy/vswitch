/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_CLI_H_
#define __VSWITCH_SRC_CLI_CLI_H_

#include <errno.h>
#include <cmdline.h>
#include <cmdline_parse.h>

extern cmdline_parse_ctx_t commands_ctx[];

/* Macros */
#define MSG_CMD_UNKNOWN         "Unknown command \"%s\".\n"
#define MSG_CMD_UNIMPLEMENTED   "Command \"%s\" not implemented.\n"
#define MSG_CMD_FAIL            "Command \"%s\" failed.\n"

#define MSG_ARG_NOT_ENOUGH      "Not enough arguments for command \"%s\".\n"
#define MSG_ARG_TOO_MANY        "Too many arguments for command \"%s\".\n"
#define MSG_ARG_MISMATCH        "Wrong number of arguments for command \"%s\".\n"
#define MSG_ARG_NOT_FOUND       "Argument \"%s\" not found.\n"
#define MSG_ARG_INVALID         "Invalid value for argument \"%s\".\n"

#endif /* __VSWITCH_SRC_CLI_CLI_H_ */
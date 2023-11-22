/*
  SPDX-License-Identifier: MIT
  Copyright(c) 2023 Sriram Yagnaraman.
*/

#ifndef __VSWITCH_SRC_CLI_CLI_H_
#define __VSWITCH_SRC_CLI_CLI_H_

#include <errno.h>
#include <cmdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>

extern cmdline_parse_ctx_t commands_ctx[];

struct quit_cmd_tokens {
	cmdline_fixed_string_t cmd;
};

/* Macros */
#define MSG_CMD_UNKNOWN         "Unknown command \"%s\".\n"
#define MSG_CMD_UNIMPLEMENTED   "Command \"%s\" not implemented.\n"
#define MSG_CMD_FAIL            "Command \"%s\" failed.\n"

#define MSG_ARG_NOT_ENOUGH      "Not enough arguments for command \"%s\".\n"
#define MSG_ARG_TOO_MANY        "Too many arguments for command \"%s\".\n"
#define MSG_ARG_MISMATCH        "Wrong number of arguments for command \"%s\".\n"
#define MSG_ARG_NOT_FOUND       "Argument \"%s\" not found.\n"
#define MSG_ARG_INVALID         "Invalid value for argument \"%s\".\n"

int cli_init(char const *prompt);
void cli_interact();
void cli_quit();
int cli_execute(const char *file_name);

#endif /* __VSWITCH_SRC_CLI_CLI_H_ */
# SPDX-License-Identifier: MIT
# Copyright(c) 2023 Sriram Yagnaraman.

name = 'vswitch-cli'
build = cc.has_header('sys/epoll.h')
if not build
    subdir_done()
endif

sources += files(
        'cli.c',
        'cli_link.c',
        'cli_mempool.c',
)

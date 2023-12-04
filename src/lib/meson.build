# SPDX-License-Identifier: MIT
# Copyright(c) 2023 Sriram Yagnaraman.

name = 'vswitch-nodes'
build = cc.has_header('sys/epoll.h')
if not build
    subdir_done()
endif

sources += files(
        'node/eventdev_rx.c',
)

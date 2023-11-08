# SPDX-License-Identifier: MIT
# Copyright(c) 2023 Sriram Yagnaraman.

name = 'vswitch'
build = cc.has_header('sys/epoll.h')
if not build
    subdir_done()
endif

sources = files(
        'cli.c',
        'conn.c',
        'ethdev_rx.c',
        'ethdev.c',
        'graph.c',
        'ip4_route.c',
        'ip6_route.c',
        'l3fwd.c',
        'main.c',
        'mempool.c',
        'neigh.c',
        'utils.c',
)

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

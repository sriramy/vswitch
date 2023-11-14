# SPDX-License-Identifier: MIT
# Copyright(c) 2023 Sriram Yagnaraman.

name = 'vswitch'
build = cc.has_header('sys/epoll.h')
if not build
    subdir_done()
endif

sources = files(
        'cli/cli.c',
        'cli/cli_link.c',
        'cli/cli_mempool.c',
        'options.c',
        'main.c',
)

includes = include_directories('api')

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

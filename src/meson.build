# SPDX-License-Identifier: MIT
# Copyright(c) 2023 Sriram Yagnaraman.

name = 'vswitch'
build = cc.has_header('sys/epoll.h')
if not build
    subdir_done()
endif

sources = files(
        'conn.c',
        'lcore.c',
        'link.c',
        'mempool.c',
        'options.c',
        'stage.c',
        'main.c',
        'vswitch.c',
)

subdir('cli')
subdir('lib')
includes = include_directories(
    'include',
    'lib',
)

deps += [dependency('threads')]
deps += [dependency('numa', required: true)]
deps += [dependency('pcap', required: true)]
deps += [cc.find_library('dl', required: false)]
deps += [cc.find_library('m', required: false)]

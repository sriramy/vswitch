# SPDX-License-Identifier: MIT
# Copyright(c) 2023 Sriram Yagnaraman.

project('vswitch', 'C',
    version: run_command(find_program('cat', 'more'),
        files('VERSION'), check:false).stdout().strip(),

	license: 'MIT',
	default_options: [
		'buildtype=release',
		'default_library=static',
		'warning_level=3',
		'werror=true'
	],
	meson_version: '>= 0.51.0'
)

vswitch_conf = configuration_data()

# set up some global vars for compiler, platform, configuration, etc.
cc = meson.get_compiler('c')

target = target_machine.cpu_family()
if (target != 'riscv64') and (target != 'aarch64')
    add_project_arguments('-march=native', language: 'c')
endif

add_project_arguments('-DALLOW_EXPERIMENTAL_API', language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')

# enable extra warnings and disable any unwanted warnings
warning_flags = [
	'-Wno-pedantic',
	'-Wno-format-truncation',
]
foreach arg: warning_flags
	if cc.has_argument(arg)
		add_project_arguments(arg, language: 'c')
	endif
endforeach

dbus = dependency('dbus-1', required: true, method: 'pkg-config', static: true)
dpdk = dependency('libdpdk', required: true, method: 'pkg-config', static: true)
dpdk_prefix = dpdk.get_variable(pkgconfig: 'prefix')
dpdk_libs_path = join_paths(dpdk_prefix, get_option('libdir'))

deps = [dpdk, dbus]

subdir('src')
executable('vswitch',
    sources: sources,
    install: true,
    dependencies: deps) 


project('fpsync', 'c',
        # Get version number from file.
        # Fallback to "more" for Windows compatibility.
        version: run_command(find_program('cat', 'more'),
            files('VERSION'), check: true).stdout().strip(),
        license: 'MIT',
        meson_version: '>= 0.53.2'
)

subdir('libnetlink')

libnetlink_dep = declare_dependency(link_with: netlink, version : '>=0.1.0')
executable('fpsync', 'main.c', dependencies: libnetlink_dep)
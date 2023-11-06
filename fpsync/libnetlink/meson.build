netlink_sources = ['netlink.c']
netlink = shared_library('netlink',
                        netlink_sources,
                        install : true)

pkg_mod = import('pkgconfig')
pkg_mod.generate(netlink,
                 version : '1.0',
                 name : 'libnetlink',
                 description : 'sync module for ip and routes using netlink')

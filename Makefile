##
## Make vswitch, dpdk based lite network stack
##
## Targets;
##  help		This printout
##  all (default)	Build all binaries
##  clean		Remove built files
##  install	Installs the executables
##  libdpdk	Builds DPDK libraries
##  libdpdk_clean Cleans DPDK build
##

LIBDPDK_SRC=dpdk/*
VSWITCH_SRC=src/*

all: vswitch

.PHONY: libdpdk
libdpdk: $(LIBDPDK_SRC)
	@echo "  libdpdk"
	$(Q)meson setup dpdk/build dpdk $(P)
	$(Q)meson configure -Dmax_lcores=4 -Denable_apps=graph dpdk/build $(P)
	$(Q)meson compile -C dpdk/build $(P)

.PHONY: libdpdk_clean
libdpdk_clean:
	$(Q)ninja -C dpdk/build -t clean $(P)

.PHONY: libdpdk_install
libdpdk_install:
	$(Q)meson install -C dpdk/build $(P)

.PHONY: vswitch
vswitch: $(VSWITCH_SRC)
	@echo "  vswitch"
	$(Q)meson setup build $(P)
	$(Q)meson configure build $(P)
	$(Q)meson compile -C build $(P)

.PHONY: clean
clean:
	$(Q)ninja -C build -t clean $(P)

.PHONY: install
install:
	$(Q)meson install -C build $(P)

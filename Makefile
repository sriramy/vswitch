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
FPSYNC_DIR=fpsync

all: fpsync

.PHONY: libdpdk
libdpdk: $(LIBDPDK_SRC)
	@echo "  libdpdk"
	$(Q)meson setup dpdk/build dpdk $(P)
	$(Q)meson configure -Dmax_lcores=4 -Dexamples=all dpdk/build $(P)
	$(Q)meson compile -C dpdk/build $(P)

.PHONY: libdpdk_clean
libdpdk_clean:
	$(Q)ninja -C dpdk/build -t clean $(P)


.PHONY: fpsync
fpsync: $(FPSYNC_DIR)
	@echo "  fpsync"
	$(Q)meson setup $(FPSYNC_DIR)/build $(FPSYNC_DIR) $(P)
	$(Q)meson configure $(FPSYNC_DIR)/build $(P)
	$(Q)meson compile -C $(FPSYNC_DIR)/build $(P)

.PHONY: fpsync_clean
fpsync_clean:
	$(Q)ninja -C $(FPSYNC_DIR)/build -t clean $(P)

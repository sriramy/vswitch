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
VSWITCH_SRC_DIR=src
CMDLINE_GEN=dpdk/buildtools/dpdk-cmdline-gen.py
CMD_LISTS := $(wildcard $(VSWITCH_SRC_DIR)/cli/*.list)
CMD_GEN_H := $(CMD_LISTS:%.list=%.h)

all: vswitch

$(VSWITCH_SRC_DIR)/cli/%.h: $(VSWITCH_SRC_DIR)/cli/%.list
	$(CMDLINE_GEN)  --output-file $@ --stubs $<

.PHONY: stubs
stubs: $(CMD_GEN_H)

.PHONY: libdpdk
libdpdk: $(LIBDPDK_SRC)
	@echo "  libdpdk"
	$(Q)meson setup dpdk/build dpdk $(P)
	$(Q)meson configure -Dmax_lcores=12 -Denable_apps=test-pmd dpdk/build $(P)
	$(Q)meson compile -C dpdk/build $(P)

.PHONY: libdpdk_clean
libdpdk_clean:
	$(Q)ninja -C dpdk/build -t clean $(P)

.PHONY: libdpdk_install
libdpdk_install:
	$(Q)meson install -C dpdk/build $(P)

.PHONY: vswitch
vswitch: $(VSWITCH_SRC_DIR)/*
	@echo "  vswitch"
	$(Q)meson setup build --buildtype debug --wipe $(P)
	$(Q)meson configure build $(P)
	$(Q)meson compile -C build $(P)

.PHONY: vsnl-interceptor
vsnl-interceptor:
	@echo "  vsnl-interceptor"
	$(Q) $(CC) -shared -fPIC -o vsnl-interceptor/vsint-sendmsg.so vsnl-interceptor/sendmsg.c -ldl

.PHONY: clean
clean:
	$(Q)ninja -C build -t clean $(P)

.PHONY: install
install:
	$(Q)meson install -C build $(P)

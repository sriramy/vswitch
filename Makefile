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

PKG_CONFIG_PATH ?= $(XCLUSTER_WORKSPACE)/sys/dpdk/usr/local/lib/x86_64-linux-gnu/pkgconfig
VSWITCH_SRC_DIR=src
CMDLINE_GEN=dpdk/buildtools/dpdk-cmdline-gen.py
CMD_LISTS := $(wildcard $(VSWITCH_SRC_DIR)/cli/*.list)
CMD_GEN_H := $(CMD_LISTS:%.list=%.h)

all: vswitch

$(VSWITCH_SRC_DIR)/cli/%.h: $(VSWITCH_SRC_DIR)/cli/%.list
	$(CMDLINE_GEN)  --output-file $@ --stubs $<

.PHONY: stubs
stubs: $(CMD_GEN_H)

.PHONY: vswitch
vswitch: $(VSWITCH_SRC_DIR)/*
	@echo "  vswitch"
	$(Q)meson setup build --buildtype debug $(P)
	$(Q)meson configure build --buildtype debug $(P)
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

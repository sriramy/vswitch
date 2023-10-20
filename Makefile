LIBDPDK_SOURCES=dpdk/*
BIN_DIR=bin/

all: build

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

.PHONY: build
build: $(BIN_DIR)

.PHONY: libdpdk
libdpdk: $(LIBDPDK_SOURCES)
	@echo "  libdpdk"
	$(Q)meson setup dpdk/build dpdk $(P)
	$(Q)meson configure -Dmax_lcores=4 -Dexamples=all dpdk/build $(P)
	$(Q)meson compile -C dpdk/build $(P)

.PHONY: libdpdk_clean
libdpdk_clean:
	$(Q)ninja -C dpdk/build -t clean $(P)

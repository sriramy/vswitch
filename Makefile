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
BIN_DIR = bin
SRC_DIR = src

CC ?= gcc
PKG_CONFIG_PATH ?= ~/xc/workspace/sys/usr/lib/pkgconfig
CFLAGS ?= -g -Wall -Wextra -I$(SRC_DIR) `PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags libmnl`
LDFLAGS ?= `PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs libmnl`

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/.%.o, $(SRC))
BINARIES = $(patsubst $(SRC_DIR)/.%.o, $(BIN_DIR)/%, $(OBJ))

all: $(BINARIES)

$(SRC_DIR)/.%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN_DIR)/%: $(SRC_DIR)/.%.o
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $< $(LDFLAGS) 

.PHONY: clean
clean:
	@rm -rf $(BINARIES) $(OBJ)

.PHONY: install
install:
	install -d $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(BINARIES) -t $(DESTDIR)/$(PREFIX)/bin

.PHONY: libdpdk
libdpdk: $(LIBDPDK_SRC)
	@echo "  libdpdk"
	$(Q)meson setup dpdk/build dpdk $(P)
	$(Q)meson configure -Dmax_lcores=4 -Dexamples=all dpdk/build $(P)
	$(Q)meson compile -C dpdk/build $(P)

.PHONY: libdpdk_clean
libdpdk_clean:
	$(Q)ninja -C dpdk/build -t clean $(P)

.PHONY: help
help:
	@grep '^##' $(lastword $(MAKEFILE_LIST)) | cut -c3-
	@echo "Binaries:"
	@echo "  $(BINARIES)"
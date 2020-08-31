.SUFFIXES:
.DEFAULT_GOAL := all

export PATH := $(shell toolchain/use-it.sh):$(PATH)
export PATH := $(shell toolbox/use-it.sh):$(PATH)

ifeq (, $(shell which inkscape))
$(error "No inkscape in PATH, consider installing it")
endif

DIRECTORY_GUARD=@mkdir -p $(@D)

BUILD_ARCH?=i686
BUILD_CONFIG?=debug
BUILD_SYSTEM?=skift

BUILD_TARGET=$(BUILD_CONFIG)-$(BUILD_ARCH)-$(BUILD_SYSTEM)
BUILD_GITREF=$(shell git rev-parse --abbrev-ref HEAD || echo unknown)@$(shell git rev-parse --short HEAD || echo unknown)
BUILD_UNAME=$(shell uname -s -o -m -r)
BUILD_DIRECTORY=$(shell pwd)/build

SYSROOT=$(BUILD_DIRECTORY)/sysroot
BOOTROOT=$(BUILD_DIRECTORY)/bootroot

BUILD_DIRECTORY_LIBS=$(SYSROOT)/System/Libraries
BUILD_DIRECTORY_INCLUDE=$(SYSROOT)/System/Includes
BUILD_DIRECTORY_APPS=$(SYSROOT)/Applications
BUILD_DIRECTORY_UTILS=$(SYSROOT)/System/Binaries

BUILD_OPTIMISATIONS?=-O2

BUILD_WARNING:= \
	-Wall \
	-Wextra  \
	-Werror \

BUILD_INCLUDE:= \
	-I. \
	-Iapplications \
	-Ilibraries \
	-Ilibraries/libc

BUILD_DEFINES:= \
	-D__BUILD_ARCH__=\""$(BUILD_ARCH)"\" \
	-D__BUILD_CONFIG__=\""$(BUILD_CONFIG)"\" \
	-D__BUILD_SYSTEM__=\""$(BUILD_SYSTEM)"\" \
	-D__BUILD_TARGET__=\""$(BUILD_TARGET)"\" \
	-D__BUILD_GITREF__=\""$(BUILD_GITREF)"\" \
	-D__BUILD_UNAME__=\""$(BUILD_UNAME)"\"

# --- Configs -------------------------------------------- #
CC:=i686-pc-skift-gcc
CFLAGS= \
	-std=gnu11 \
	-MD \
	$(BUILD_OPTIMISATIONS) \
	$(BUILD_WARNING) \
	$(BUILD_INCLUDE) \
	$(BUILD_DEFINES)

CXX:=i686-pc-skift-g++
CXXFLAGS:= \
	-std=c++20 \
	-fno-rtti \
	-fno-exceptions \
	-MD \
	$(BUILD_OPTIMISATIONS) \
	$(BUILD_WARNING) \
	$(BUILD_INCLUDE) \
	$(BUILD_DEFINES)

LD:=i686-pc-skift-ld
LDFLAGS:=

AR:=i686-pc-skift-ar
ARFLAGS:=rcs

AS=nasm
ASFLAGS=-f elf32

include kernel/.build.mk
include libraries/.build.mk
include coreutils/.build.mk
include applications/.build.mk
include icons/.build.mk

# --- Ramdisk -------------------------------------------- #

RAMDISK=$(BOOTROOT)/boot/ramdisk.tar

SYSROOT_CONTENT=$(shell find sysroot/ -type f)

$(RAMDISK): $(CRTS) $(TARGETS) $(HEADERS) $(SYSROOT_CONTENT)
	$(DIRECTORY_GUARD)

	@echo [TAR] $@

	@mkdir -p \
		$(SYSROOT)/Applications \
		$(SYSROOT)/Devices \
		$(SYSROOT)/Session \
		$(SYSROOT)/System \
		$(SYSROOT)/System/Binaries \
		$(SYSROOT)/System/Configs \
		$(SYSROOT)/System/Fonts \
		$(SYSROOT)/System/Icons \
		$(SYSROOT)/System/Includes \
		$(SYSROOT)/System/Libraries \
		$(SYSROOT)/User \
		$(SYSROOT)/User/Applications \
		$(SYSROOT)/User/Configs \
		$(SYSROOT)/User/Desktop \
		$(SYSROOT)/User/Documents \
		$(SYSROOT)/User/Download  \
		$(SYSROOT)/User/Movies  \
		$(SYSROOT)/User/Music  \
		$(SYSROOT)/User/Pictures  \
		$(SYSROOT)/User/Public  \
		$(SYSROOT)/User/Sites

	@cp -r sysroot/* $(SYSROOT)/

	@cd $(SYSROOT); tar -cf $@ *

# --- Bootdisk ------------------------------------------- #

BOOTDISK=$(BUILD_DIRECTORY)/bootdisk.iso

$(BOOTDISK): $(RAMDISK) $(KERNEL_BINARY) grub.cfg
	$(DIRECTORY_GUARD)
	@echo [GRUB-MKRESCUE] $@

	@mkdir -p $(BOOTROOT)/boot/grub
	@cp grub.cfg $(BOOTROOT)/boot/grub/

	@grub-mkrescue -o $@ $(BOOTROOT) || \
	 grub2-mkrescue -o $@ $(BOOTROOT)

# --- Phony ---------------------------------------------- #

.PHONY: all
all: $(BOOTDISK)

.PHONY: run
run: run-qemu

VM_MEMORY?=128

QEMU=qemu-system-x86_64
QEMU_FLAGS=-m $(VM_MEMORY)M \
		  -serial stdio \
		  -rtc base=localtime

QEMU_FLAGS_VIRTIO=-device virtio-rng-pci \
				 -device virtio-serial \
				 -nic user,model=virtio-net-pci
#				 -vga virtio

.PHONY: run-qemu
run-qemu: $(BOOTDISK)
	@echo [QEMU] $^
	$(QEMU) -cdrom $^ $(QEMU_FLAGS) $(QEMU_EXTRA) -enable-kvm || \
	$(QEMU) -cdrom $^ $(QEMU_FLAGS) $(QEMU_EXTRA)

run-qemu-virtio: $(BOOTDISK)
	@echo [QEMU] $^
	$(QEMU) -cdrom $^ $(QEMU_FLAGS) $(QEMU_FLAGS_VIRTIO) $(QEMU_EXTRA) -enable-kvm

.PHONY: run-vbox
run-vbox: $(BOOTDISK)
	@VBoxManage unregistervm --delete "skiftOS-dev" || echo "Look like it's the fist time you are running this command, this is fine"
	@VBoxManage createvm \
		--name skiftOS-dev \
		--ostype Other \
		--register \
		--basefolder $(shell pwd)/vm

	@VBoxManage modifyvm \
		skiftOS-dev \
		--memory $(VM_MEMORY) \
		--uart1 0x3F8 4 \
		--uartmode1 tcpserver 1234

	@VBoxManage storagectl \
		skiftOS-dev \
		--name IDE \
		--add ide \

	@VBoxManage storageattach \
		skiftOS-dev \
		--storagectl IDE \
		--port 0 \
		--device 0 \
		--type dvddrive \
		--medium $(BOOTDISK)

ifeq ($(VBOX_DISPLAY),sdl)
	@vboxsdl --startvm skiftOS-dev &
	@sleep 1
else
	@VBoxManage startvm skiftOS-dev --type gui
endif
	nc localhost 1234


.PHONY: sync
sync:
	rm $(BOOTDISK) $(RAMDISK)
	make $(BOOTDISK)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIRECTORY)

clean-fs:
	rm -rf $(SYSROOT)

.PHONY: install-headers
install-headers: $(HEADERS)

-include $(OBJECTS:.o=.d)

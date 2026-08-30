SHELL := /bin/bash

# =========================================================
# MyOS final build system
# =========================================================
#
# Normal development workflow:
#
#   make dev
#
# Add userspace source files to:
#
#   src/programs/*.c
#   src/programs/*.cpp
#
# They are compiled once into build/programs/ and the same
# binaries are synchronized into the staged final rootfs.
#
# The development initramfs is built FROM the staged rootfs,
# never from the installer initramfs. This keeps development
# completely separate from disk installation tooling.
#
# =========================================================


# =========================================================
# HOST TOOLS
# =========================================================

BUSYBOX      ?= /usr/bin/busybox
MKFS_EXT4    ?= /usr/sbin/mkfs.ext4
GRUB_INSTALL ?= /usr/sbin/grub-install

CC           ?= gcc
CXX          ?= g++

QEMU         ?= qemu-system-x86_64
QEMU_IMG     ?= qemu-img


# =========================================================
# PROJECT PATHS
# =========================================================

KERNEL := rootfs/boot/vmlinuz

PROGRAM_SRC_DIR   := src/programs
PROGRAM_BUILD_DIR := build/programs

DEV_INITRAMFS_DIR := build/dev-initramfs

INSTALLER_DIR := installer
INITRAMFS_DIR := initramfs
ISO_DIR       := iso
OUTPUT_DIR    := output

INSTALLER_INITRAMFS := $(INSTALLER_DIR)/initramfs.img
DEV_INITRAMFS       := $(INSTALLER_DIR)/dev-initramfs.img
ROOTFS_ARCHIVE      := $(INSTALLER_DIR)/rootfs.tar.gz

ISO_IMAGE  := $(OUTPUT_DIR)/myos.iso
DISK_IMAGE := $(OUTPUT_DIR)/myos-disk.img


# =========================================================
# DESKTOP PROJECT
# =========================================================

DESKTOP_SRC_DIR   := src/desktop
DESKTOP_BUILD_DIR := build/desktop
DESKTOP_BINARY    := $(DESKTOP_BUILD_DIR)/myos-desktop

DESKTOP_SOURCES := $(shell find $(DESKTOP_SRC_DIR) -type f -name '*.cpp' 2>/dev/null)

DESKTOP_OBJECTS := \
	$(patsubst $(DESKTOP_SRC_DIR)/%.cpp,$(DESKTOP_BUILD_DIR)/%.o,$(DESKTOP_SOURCES))


# =========================================================
# USERSPACE PROGRAM DISCOVERY
# =========================================================

C_SOURCES   := $(wildcard $(PROGRAM_SRC_DIR)/*.c)
CPP_SOURCES := $(wildcard $(PROGRAM_SRC_DIR)/*.cpp)

C_PROGRAMS := \
	$(patsubst $(PROGRAM_SRC_DIR)/%.c,$(PROGRAM_BUILD_DIR)/%,$(C_SOURCES))

CPP_PROGRAMS := \
	$(patsubst $(PROGRAM_SRC_DIR)/%.cpp,$(PROGRAM_BUILD_DIR)/%,$(CPP_SOURCES))

PROGRAMS := $(sort $(C_PROGRAMS) $(CPP_PROGRAMS))


# =========================================================
# COMPILER FLAGS
# =========================================================

CFLAGS   ?= -static -O2 -Wall -Wextra
CXXFLAGS ?= -static -std=c++17 -O2 -Wall -Wextra
DESKTOP_CXXFLAGS := \
	-static \
	-std=c++17 \
	-O2 \
	-Wall \
	-Wextra \
	-I$(DESKTOP_SRC_DIR)


# =========================================================
# QEMU SETTINGS
# =========================================================

QEMU_MEMORY ?= 128M

QEMU_COMMON := \
	-m $(QEMU_MEMORY) \
	-serial stdio


# =========================================================
# TARGET DECLARATIONS
# =========================================================

.PHONY: \
	all \
	check_host \
	programs build_programs clean_programs \
	prepare_rootfs sync_rootfs_programs validate_rootfs \
	installer \
	prepare_installer_initramfs \
	copy_busybox copy_mkfs_ext4 copy_grub \
	install_initramfs_programs \
	build_initramfs_img \
	build_dev_initramfs_img \
	dev dev_shell dev_gui \
	boot run \
	iso_structure clean_iso build_iso \
	disk recreate_disk clean_disk \
	boot_iso test_disk \
	clean distclean \
	desktop build_desktop clean_desktop


# =========================================================
# DEFAULT BUILD
# =========================================================

all: installer build_initramfs_img
	@echo
	@echo "[+] MyOS build complete."
	@echo "    Rootfs : $(ROOTFS_ARCHIVE)"
	@echo "    Initrd : $(INSTALLER_INITRAMFS)"


# =========================================================
# HOST VALIDATION
# =========================================================

check_host:
	@command -v $(CC) >/dev/null 2>&1 || \
		(echo "ERROR: C compiler not found: $(CC)"; exit 1)

	@command -v $(CXX) >/dev/null 2>&1 || \
		(echo "ERROR: C++ compiler not found: $(CXX)"; exit 1)

	@test -x "$(BUSYBOX)" || \
		(echo "ERROR: BusyBox not found: $(BUSYBOX)"; exit 1)

	@test -x "$(MKFS_EXT4)" || \
		(echo "ERROR: mkfs.ext4 not found: $(MKFS_EXT4)"; exit 1)

	@test -x "$(GRUB_INSTALL)" || \
		(echo "ERROR: grub-install not found: $(GRUB_INSTALL)"; exit 1)

	@test -f "$(KERNEL)" || \
		(echo "ERROR: kernel not found: $(KERNEL)"; exit 1)


# =========================================================
# USERSPACE PROGRAMS
# =========================================================

programs: build_programs


build_programs: check_host $(PROGRAMS)
	@echo "[+] Userspace programs ready."

	@if [ -n "$(PROGRAMS)" ]; then \
		echo "[+] Built programs:"; \
		for program in $(PROGRAMS); do \
			echo "    $$program"; \
		done; \
	else \
		echo "[+] No C/C++ programs found in $(PROGRAM_SRC_DIR)"; \
	fi


$(PROGRAM_BUILD_DIR)/%: $(PROGRAM_SRC_DIR)/%.c
	@echo "[+] Building C userspace program: $*"

	@mkdir -p "$(PROGRAM_BUILD_DIR)"

	$(CC) $(CFLAGS) "$<" -o "$@"

	@chmod 755 "$@"

	@echo "[+] Built: $@"


$(PROGRAM_BUILD_DIR)/%: $(PROGRAM_SRC_DIR)/%.cpp
	@echo "[+] Building C++ userspace program: $*"

	@mkdir -p "$(PROGRAM_BUILD_DIR)"

	$(CXX) $(CXXFLAGS) "$<" -o "$@"

	@chmod 755 "$@"

	@echo "[+] Built: $@"


clean_programs:
	rm -rf "$(PROGRAM_BUILD_DIR)"

# =========================================================
# MYOS DESKTOP
# =========================================================

desktop: build_desktop


build_desktop: check_host $(DESKTOP_BINARY)
	@echo "[+] MyOS desktop ready."
	@echo "    $(DESKTOP_BINARY)"


$(DESKTOP_BUILD_DIR)/%.o: $(DESKTOP_SRC_DIR)/%.cpp
	@echo "[+] Compiling desktop source: $<"

	@mkdir -p "$(dir $@)"

	$(CXX) $(DESKTOP_CXXFLAGS) -c "$<" -o "$@"


$(DESKTOP_BINARY): $(DESKTOP_OBJECTS)
	@echo "[+] Linking MyOS desktop..."

	@mkdir -p "$(DESKTOP_BUILD_DIR)"

	$(CXX) -static $(DESKTOP_OBJECTS) -o "$(DESKTOP_BINARY)"

	@chmod 755 "$(DESKTOP_BINARY)"

	@echo "[+] Built: $(DESKTOP_BINARY)"


clean_desktop:
	rm -rf "$(DESKTOP_BUILD_DIR)"

# =========================================================
# INSTALLED ROOT FILESYSTEM
# =========================================================
#
# IMPORTANT:
# This target does NOT erase rootfs/.
#
# rootfs/ contains your staged MyOS filesystem and kernel.
# Rebuilding userspace must never destroy it.
#
# =========================================================

prepare_rootfs: check_host
	@echo "[+] Preparing installed root filesystem..."

	@mkdir -p \
		rootfs/bin \
		rootfs/sbin \
		rootfs/etc \
		rootfs/etc/init.d \
		rootfs/etc/udhcpc \
		rootfs/etc/dropbear \
		rootfs/proc \
		rootfs/sys \
		rootfs/dev \
		rootfs/tmp \
		rootfs/run \
		rootfs/mnt \
		rootfs/root \
		rootfs/home \
		rootfs/var \
		rootfs/var/log \
		rootfs/var/run \
		rootfs/var/lib \
		rootfs/var/tmp \
		rootfs/usr \
		rootfs/usr/bin \
		rootfs/usr/sbin \
		rootfs/usr/lib

	@echo "[+] Installing BusyBox into rootfs..."

	@rm -f rootfs/bin/busybox
	@cp -L "$(BUSYBOX)" rootfs/bin/busybox
	@chmod 755 rootfs/bin/busybox

	@echo "[+] Creating BusyBox applet links..."

	# copy default dhcp script into udhcpc
	@cp src/default-udhcpc-script rootfs/etc/udhcpc/default.script
	# copy dropbear (ssh)
	@cp src/third_party/dropbear/dropbear rootfs/usr/sbin/dropbear
	# copy dropbearkey (ssh)
	@cp src/third_party/dropbear/dropbearkey rootfs/usr/bin/dropbearkey

	# set permissions for ssh packages
	chmod 755 rootfs/usr/sbin/dropbear
	chmod 755 rootfs/usr/bin/dropbearkey
	chmod 700 rootfs/etc/dropbear

	@cd rootfs/bin && \
	for applet in $$(./busybox --list); do \
		[ "$$applet" = "busybox" ] && continue; \
		ln -sf busybox "$$applet"; \
	done

	@rm -f rootfs/bin/sh
	@ln -s busybox rootfs/bin/sh

	@rm -f rootfs/sbin/init
	@ln -s ../bin/busybox rootfs/sbin/init

	@echo "[+] Normalizing rootfs configuration files..."

	@if [ -f rootfs/etc/inittab ]; then \
		sed -i 's/\r$$//' rootfs/etc/inittab; \
		chmod 644 rootfs/etc/inittab; \
	fi

	@if [ -f rootfs/etc/init.d/rcS ]; then \
		sed -i 's/\r$$//' rootfs/etc/init.d/rcS; \
		chmod 755 rootfs/etc/init.d/rcS; \
	fi

	@if [ -f rootfs/etc/hostname ]; then \
		sed -i 's/\r$$//' rootfs/etc/hostname; \
		chmod 644 rootfs/etc/hostname; \
	fi

	@if [ -f rootfs/etc/issue ]; then \
		sed -i 's/\r$$//' rootfs/etc/issue; \
		chmod 644 rootfs/etc/issue; \
	fi

	@if [ -f rootfs/etc/passwd ]; then \
		sed -i 's/\r$$//' rootfs/etc/passwd; \
		chmod 644 rootfs/etc/passwd; \
	fi

	@if [ -f rootfs/etc/group ]; then \
		sed -i 's/\r$$//' rootfs/etc/group; \
		chmod 644 rootfs/etc/group; \
	fi

	@if [ -f rootfs/etc/shadow ]; then \
		sed -i 's/\r$$//' rootfs/etc/shadow; \
		chmod 600 rootfs/etc/shadow; \
	fi

	@if [ -f rootfs/etc/gshadow ]; then \
		sed -i 's/\r$$//' rootfs/etc/gshadow; \
		chmod 600 rootfs/etc/gshadow; \
	fi

	@if [ -f rootfs/etc/udhcpc/default.script ]; then \
		sed -i 's/\r$$//' rootfs/etc/udhcpc/default.script; \
		chmod 755 rootfs/etc/udhcpc/default.script; \
	fi

	@chmod 1777 rootfs/tmp
	@chmod 1777 rootfs/var/tmp
	@chmod 700 rootfs/root
	@chmod 755 rootfs/home

	@echo "[+] Installed rootfs base prepared."


# =========================================================
# COPY COMPILED PROGRAMS INTO ROOTFS
# =========================================================

sync_rootfs_programs: prepare_rootfs build_programs build_desktop
	@echo "[+] Synchronizing MyOS programs into rootfs/usr/bin..."

	@mkdir -p rootfs/usr/bin

	@if [ -n "$(PROGRAMS)" ]; then \
		for program in $(PROGRAMS); do \
			name=$$(basename "$$program"); \
			cp -a "$$program" "rootfs/usr/bin/$$name"; \
			chmod 755 "rootfs/usr/bin/$$name"; \
		done; \
	fi

	@if [ -x "$(DESKTOP_BINARY)" ]; then \
		echo "[+] Installing MyOS desktop..."; \
		cp -a "$(DESKTOP_BINARY)" rootfs/usr/bin/myos-desktop; \
		chmod 755 rootfs/usr/bin/myos-desktop; \
	fi

	@echo "[+] Rootfs userspace programs synchronized."


# =========================================================
# VALIDATE ROOTFS
# =========================================================

validate_rootfs: sync_rootfs_programs
	@echo "[+] Validating installed rootfs..."

	@test -x rootfs/bin/busybox || \
		(echo "ERROR: rootfs/bin/busybox missing"; exit 1)

	@test ! -L rootfs/bin/busybox || \
		(echo "ERROR: rootfs/bin/busybox must be a real binary"; exit 1)

	@test -L rootfs/bin/sh || \
		(echo "ERROR: rootfs/bin/sh missing"; exit 1)

	@test "$$(readlink rootfs/bin/sh)" = "busybox" || \
		(echo "ERROR: rootfs/bin/sh must point to busybox"; exit 1)

	@test -L rootfs/bin/cat || \
		(echo "ERROR: rootfs/bin/cat missing"; exit 1)

	@test "$$(readlink rootfs/bin/cat)" = "busybox" || \
		(echo "ERROR: rootfs/bin/cat must point to busybox"; exit 1)

	@test -L rootfs/sbin/init || \
		(echo "ERROR: rootfs/sbin/init must be a symlink"; exit 1)

	@test "$$(readlink rootfs/sbin/init)" = "../bin/busybox" || \
		(echo "ERROR: rootfs/sbin/init must point to ../bin/busybox"; exit 1)

	@test -f rootfs/etc/inittab || \
		(echo "ERROR: rootfs/etc/inittab missing"; exit 1)

	@test -x rootfs/etc/init.d/rcS || \
		(echo "ERROR: rootfs/etc/init.d/rcS missing"; exit 1)

	@test -f rootfs/etc/passwd || \
		(echo "ERROR: rootfs/etc/passwd missing"; exit 1)

	@test -f rootfs/etc/group || \
		(echo "ERROR: rootfs/etc/group missing"; exit 1)

	@test -f rootfs/etc/shadow || \
		(echo "ERROR: rootfs/etc/shadow missing"; exit 1)

	@for program in $(PROGRAMS); do \
		name=$$(basename "$$program"); \
		test -x "rootfs/usr/bin/$$name" || { \
			echo "ERROR: rootfs program $$name missing"; \
			exit 1; \
		}; \
	done

	@echo "[+] rootfs validation OK"
	@echo "    /bin/busybox = real binary"
	@echo "    /bin/sh      -> busybox"
	@echo "    /bin/cat     -> busybox"
	@echo "    /sbin/init   -> ../bin/busybox"

	@if [ -n "$(PROGRAMS)" ]; then \
		echo "    MyOS programs installed"; \
	fi


# =========================================================
# CREATE FINAL ROOTFS PAYLOAD
# =========================================================

installer: validate_rootfs
	@echo "[+] Building rootfs.tar.gz..."

	@mkdir -p "$(INSTALLER_DIR)"

	sudo tar \
		--numeric-owner \
		--owner=0 \
		--group=0 \
		-czpf "$(ROOTFS_ARCHIVE)" \
		-C rootfs .

	@echo "[+] $(ROOTFS_ARCHIVE) created."


# =========================================================
# PREPARE INSTALLER INITRAMFS
# =========================================================
#
# This tree is installer-only.
# It may partition disks and install GRUB.
#
# =========================================================

prepare_installer_initramfs: check_host build_programs
	@echo "[+] Preparing installer initramfs..."

	@rm -rf "$(INITRAMFS_DIR)"
	@mkdir -p \
		"$(INITRAMFS_DIR)/bin" \
		"$(INITRAMFS_DIR)/sbin" \
		"$(INITRAMFS_DIR)/etc" \
		"$(INITRAMFS_DIR)/proc" \
		"$(INITRAMFS_DIR)/sys" \
		"$(INITRAMFS_DIR)/dev" \
		"$(INITRAMFS_DIR)/tmp" \
		"$(INITRAMFS_DIR)/run" \
		"$(INITRAMFS_DIR)/mnt" \
		"$(INITRAMFS_DIR)/newroot" \
		"$(INITRAMFS_DIR)/media/iso" \
		"$(INITRAMFS_DIR)/usr/bin" \
		"$(INITRAMFS_DIR)/usr/sbin" \
		"$(INITRAMFS_DIR)/usr/lib/grub" \
		"$(INITRAMFS_DIR)/lib/x86_64-linux-gnu" \
		"$(INITRAMFS_DIR)/lib64"

	@cp src/initramfs-init "$(INITRAMFS_DIR)/init"

	@cp src/install-rootfs-sh "$(INITRAMFS_DIR)/bin/install-rootfs"
	@cp src/prepare-disk-sh  "$(INITRAMFS_DIR)/bin/prepare-disk"
	@cp src/grub-setup-sh    "$(INITRAMFS_DIR)/bin/grub-setup"

	@sed -i 's/\r$$//' "$(INITRAMFS_DIR)/init"
	@sed -i 's/\r$$//' "$(INITRAMFS_DIR)/bin/install-rootfs"
	@sed -i 's/\r$$//' "$(INITRAMFS_DIR)/bin/prepare-disk"
	@sed -i 's/\r$$//' "$(INITRAMFS_DIR)/bin/grub-setup"

	@chmod 755 "$(INITRAMFS_DIR)/init"
	@chmod 755 "$(INITRAMFS_DIR)/bin/install-rootfs"
	@chmod 755 "$(INITRAMFS_DIR)/bin/prepare-disk"
	@chmod 755 "$(INITRAMFS_DIR)/bin/grub-setup"

	@echo "[+] Installer initramfs structure prepared."


# Backward-compatible alias for the old target name.
prepare: prepare_installer_initramfs


# =========================================================
# INSTALLER BUSYBOX
# =========================================================

copy_busybox: prepare_installer_initramfs
	@echo "[+] Copying BusyBox into installer initramfs..."

	@cp -L "$(BUSYBOX)" "$(INITRAMFS_DIR)/bin/busybox"
	@chmod 755 "$(INITRAMFS_DIR)/bin/busybox"

	@cd "$(INITRAMFS_DIR)/bin" && \
	for applet in $$(./busybox --list); do \
		[ "$$applet" = "busybox" ] && continue; \
		ln -sf busybox "$$applet"; \
	done

	@rm -f "$(INITRAMFS_DIR)/bin/sh"
	@ln -s busybox "$(INITRAMFS_DIR)/bin/sh"

	@echo "[+] Installer BusyBox ready."


# =========================================================
# EXT4 TOOLS
# =========================================================

copy_mkfs_ext4: copy_busybox
	@echo "[+] Copying mkfs.ext4..."

	@mkdir -p "$(INITRAMFS_DIR)/usr/sbin"

	@cp -L "$(MKFS_EXT4)" "$(INITRAMFS_DIR)/usr/sbin/mkfs.ext4"
	@chmod 755 "$(INITRAMFS_DIR)/usr/sbin/mkfs.ext4"

	@echo "[+] Copying mkfs.ext4 shared libraries..."

	@ldd "$(MKFS_EXT4)" | awk '\
		/=> \// {print $$3} \
		/^[[:space:]]*\// {print $$1} \
	' | while read -r lib; do \
		dest="$(INITRAMFS_DIR)$$(dirname "$$lib")"; \
		mkdir -p "$$dest"; \
		cp -L "$$lib" "$$dest/"; \
		echo "    $$lib"; \
	done

	@test -x "$(INITRAMFS_DIR)/usr/sbin/mkfs.ext4" || \
		(echo "ERROR: mkfs.ext4 was not copied"; exit 1)

	@test -f "$(INITRAMFS_DIR)/lib64/ld-linux-x86-64.so.2" || \
		(echo "ERROR: dynamic loader missing"; exit 1)

	@echo "[+] mkfs.ext4 ready."


# =========================================================
# GRUB INSTALLER
# =========================================================

copy_grub: copy_mkfs_ext4
	@echo "[+] Copying grub-install..."

	@mkdir -p "$(INITRAMFS_DIR)/usr/sbin"
	@mkdir -p "$(INITRAMFS_DIR)/usr/lib/grub"

	@cp -L "$(GRUB_INSTALL)" "$(INITRAMFS_DIR)/usr/sbin/grub-install"
	@chmod 755 "$(INITRAMFS_DIR)/usr/sbin/grub-install"

	@rm -rf "$(INITRAMFS_DIR)/usr/lib/grub/i386-pc"
	@cp -a /usr/lib/grub/i386-pc "$(INITRAMFS_DIR)/usr/lib/grub/"

	@echo "[+] Copying grub-install shared libraries..."

	@ldd "$(GRUB_INSTALL)" | awk '\
		/=> \// {print $$3} \
		/^[[:space:]]*\// {print $$1} \
	' | while read -r lib; do \
		dest="$(INITRAMFS_DIR)$$(dirname "$$lib")"; \
		mkdir -p "$$dest"; \
		cp -L "$$lib" "$$dest/"; \
		echo "    $$lib"; \
	done

	@test -x "$(INITRAMFS_DIR)/usr/sbin/grub-install" || \
		(echo "ERROR: grub-install was not copied"; exit 1)

	@test -d "$(INITRAMFS_DIR)/usr/lib/grub/i386-pc" || \
		(echo "ERROR: GRUB i386-pc modules missing"; exit 1)

	@echo "[+] GRUB installer ready."


# =========================================================
# COPY MYOS PROGRAMS INTO INSTALLER INITRAMFS
# =========================================================
#
# Kept intentionally so the same userspace binaries are also
# available when debugging the installer environment.
#
# =========================================================

install_initramfs_programs: copy_grub build_programs
	@echo "[+] Installing MyOS programs into installer initramfs..."

	@mkdir -p "$(INITRAMFS_DIR)/usr/bin"

	@if [ -n "$(PROGRAMS)" ]; then \
		for program in $(PROGRAMS); do \
			name=$$(basename "$$program"); \
			cp -a "$$program" "$(INITRAMFS_DIR)/usr/bin/$$name"; \
			chmod 755 "$(INITRAMFS_DIR)/usr/bin/$$name"; \
		done; \
	fi

	@for program in $(PROGRAMS); do \
		name=$$(basename "$$program"); \
		test -x "$(INITRAMFS_DIR)/usr/bin/$$name" || { \
			echo "ERROR: installer initramfs program $$name missing"; \
			exit 1; \
		}; \
	done

	@echo "[+] Installer userspace programs ready."


# =========================================================
# BUILD INSTALLER INITRAMFS
# =========================================================

build_initramfs_img: install_initramfs_programs
	@echo "[+] Validating installer initramfs..."

	@test -x "$(INITRAMFS_DIR)/init" || \
		(echo "ERROR: installer /init missing"; exit 1)

	@test -x "$(INITRAMFS_DIR)/bin/busybox" || \
		(echo "ERROR: installer BusyBox missing"; exit 1)

	@test ! -L "$(INITRAMFS_DIR)/bin/busybox" || \
		(echo "ERROR: installer BusyBox must be a real binary"; exit 1)

	@test -L "$(INITRAMFS_DIR)/bin/sh" || \
		(echo "ERROR: installer /bin/sh missing"; exit 1)

	@test "$$(readlink "$(INITRAMFS_DIR)/bin/sh")" = "busybox" || \
		(echo "ERROR: installer /bin/sh invalid"; exit 1)

	@test -x "$(INITRAMFS_DIR)/bin/install-rootfs" || \
		(echo "ERROR: install-rootfs missing"; exit 1)

	@test -x "$(INITRAMFS_DIR)/bin/prepare-disk" || \
		(echo "ERROR: prepare-disk missing"; exit 1)

	@test -x "$(INITRAMFS_DIR)/bin/grub-setup" || \
		(echo "ERROR: grub-setup missing"; exit 1)

	@test -x "$(INITRAMFS_DIR)/usr/sbin/mkfs.ext4" || \
		(echo "ERROR: mkfs.ext4 missing"; exit 1)

	@test -x "$(INITRAMFS_DIR)/usr/sbin/grub-install" || \
		(echo "ERROR: grub-install missing"; exit 1)

	@echo "[+] Installer initramfs validation OK"

	@mkdir -p "$(INSTALLER_DIR)"

	@echo "[+] Building $(INSTALLER_INITRAMFS)..."

	@cd "$(INITRAMFS_DIR)" && \
	find . -print0 \
		| cpio --null -ov --format=newc \
		| gzip -9 > "../$(INSTALLER_INITRAMFS)"

	@test -f "$(INSTALLER_INITRAMFS)" || \
		(echo "ERROR: installer initramfs was not created"; exit 1)

	@echo "[+] $(INSTALLER_INITRAMFS) ready."


# =========================================================
# DEVELOPMENT INITRAMFS
# =========================================================
#
# IMPORTANT:
#
# Development boot is built FROM THE FINAL ROOTFS STAGING TREE,
# not from the installer initramfs.
#
# Therefore:
#
#   * no prepare-disk
#   * no mkfs.ext4
#   * no grub-install
#   * no installer /init
#   * no virtual disk required
#
# It contains the same MyOS userspace that will be installed.
#
# =========================================================

build_dev_initramfs_img: validate_rootfs
	@echo "[+] Building MyOS development initramfs..."

	@rm -rf "$(DEV_INITRAMFS_DIR)"
	@mkdir -p "$(DEV_INITRAMFS_DIR)"

	@cp -a rootfs/. "$(DEV_INITRAMFS_DIR)/"

	# The kernel is already supplied directly by QEMU.
	# Do not duplicate /boot contents inside the development initramfs.
	@rm -rf "$(DEV_INITRAMFS_DIR)/boot"

	@mkdir -p \
		"$(DEV_INITRAMFS_DIR)/proc" \
		"$(DEV_INITRAMFS_DIR)/sys" \
		"$(DEV_INITRAMFS_DIR)/dev" \
		"$(DEV_INITRAMFS_DIR)/run" \
		"$(DEV_INITRAMFS_DIR)/tmp" \
		"$(DEV_INITRAMFS_DIR)/mnt"

	@cp src/dev-init "$(DEV_INITRAMFS_DIR)/init"
	@sed -i 's/\r$$//' "$(DEV_INITRAMFS_DIR)/init"
	@chmod 755 "$(DEV_INITRAMFS_DIR)/init"

	@test -x "$(DEV_INITRAMFS_DIR)/bin/busybox" || \
		(echo "ERROR: development BusyBox missing"; exit 1)

	@test -x "$(DEV_INITRAMFS_DIR)/init" || \
		(echo "ERROR: development /init missing"; exit 1)

	@for program in $(PROGRAMS); do \
		name=$$(basename "$$program"); \
		test -x "$(DEV_INITRAMFS_DIR)/usr/bin/$$name" || { \
			echo "ERROR: development program $$name missing"; \
			exit 1; \
		}; \
	done

	@mkdir -p "$(INSTALLER_DIR)"

	@cd "$(DEV_INITRAMFS_DIR)" && \
	find . -print0 \
		| cpio --null -ov --format=newc \
		| gzip -9 > "../../$(DEV_INITRAMFS)"

	@test -f "$(DEV_INITRAMFS)" || \
		(echo "ERROR: development initramfs was not created"; exit 1)

	@echo "[+] Development initramfs ready:"
	@echo "    $(DEV_INITRAMFS)"


# =========================================================
# FAST DEVELOPMENT BOOT
# =========================================================

dev: build_dev_initramfs_img
	@echo
	@echo "[+] Booting MyOS development environment..."
	@echo "[+] Development boot has NO virtual disk attached."
	@echo "[+] Installer code cannot modify $(DISK_IMAGE)."
	@echo

	$(QEMU) \
		-m $(QEMU_MEMORY) \
		-kernel "$(KERNEL)" \
		-initrd "$(DEV_INITRAMFS)" \
		-append "console=ttyS0 rdinit=/init" \
		-netdev user,id=myosnet,hostfwd=tcp:127.0.0.1:2222-:22 \
		-device virtio-net-pci,netdev=myosnet \
        -nographic

dev_shell: dev

# =========================================================
# GRAPHICAL DEVELOPMENT BOOT
# =========================================================

dev_gui: build_dev_initramfs_img
	@echo
	@echo "[+] Booting MyOS graphical development environment..."
	@echo "[+] Serial console remains available in this terminal."
	@echo

	$(QEMU) \
		-m $(QEMU_MEMORY) \
		-kernel "$(KERNEL)" \
		-initrd "$(DEV_INITRAMFS)" \
		-append "console=ttyS0 rdinit=/init" \
		-netdev user,id=myosnet,hostfwd=tcp:127.0.0.1:2222-:22 \
		-device virtio-net-pci,netdev=myosnet \
		-device VGA \
		-display gtk \
		-serial stdio


# =========================================================
# DIRECT INSTALLER INITRAMFS TEST
# =========================================================
#
# Also diskless. Useful only for testing installer initramfs
# startup itself. Full installation testing is make boot_iso.
#
# =========================================================

boot: build_initramfs_img
	$(QEMU) \
		-m $(QEMU_MEMORY) \
		-kernel "$(KERNEL)" \
		-initrd "$(INSTALLER_INITRAMFS)" \
		-append "console=ttyS0 rdinit=/init" \
		-nographic


run: installer build_initramfs_img
	$(MAKE) boot


# =========================================================
# ISO STRUCTURE
# =========================================================

iso_structure: installer build_initramfs_img
	@echo "[+] Creating ISO filesystem..."

	@rm -rf "$(ISO_DIR)"

	@mkdir -p "$(ISO_DIR)/boot/grub"
	@mkdir -p "$(ISO_DIR)/system"

	@cp "$(KERNEL)" \
		"$(ISO_DIR)/boot/vmlinuz"

	@cp "$(INSTALLER_INITRAMFS)" \
		"$(ISO_DIR)/boot/initramfs.img"

	@cp "$(ROOTFS_ARCHIVE)" \
		"$(ISO_DIR)/system/rootfs.tar.gz"

	@cp src/grub-grub-cfg \
		"$(ISO_DIR)/boot/grub/grub.cfg"

	@echo "[+] ISO filesystem ready."


clean_iso:
	rm -rf "$(ISO_DIR)"


# =========================================================
# BUILD BOOTABLE ISO
# =========================================================
#
# Rebuilding the ISO NEVER deletes the virtual hard disk.
#
# =========================================================

build_iso: iso_structure
	@echo "[+] Building bootable ISO..."

	@mkdir -p "$(OUTPUT_DIR)"
	@rm -f "$(ISO_IMAGE)"

	grub-mkrescue \
		-o "$(ISO_IMAGE)" \
		"$(ISO_DIR)"

	@$(MAKE) clean_iso

	@echo
	@echo "[+] ISO build complete"
	@echo "    ISO : $(ISO_IMAGE)"


# =========================================================
# VIRTUAL INSTALLATION DISK
# =========================================================
#
# make disk:
#   create only if it does not exist.
#
# make recreate_disk:
#   explicitly destroy and recreate it.
#
# =========================================================

disk:
	@mkdir -p "$(OUTPUT_DIR)"

	@if [ -f "$(DISK_IMAGE)" ]; then \
		echo "[+] Existing virtual disk preserved:"; \
		echo "    $(DISK_IMAGE)"; \
	else \
		echo "[+] Creating virtual installation disk..."; \
		$(QEMU_IMG) create -f raw "$(DISK_IMAGE)" 1G; \
	fi


recreate_disk:
	@echo "[+] Recreating virtual installation disk..."
	@rm -f "$(DISK_IMAGE)"
	@$(MAKE) disk


clean_disk:
	rm -f "$(DISK_IMAGE)"


# =========================================================
# BOOT INSTALLER ISO
# =========================================================

boot_iso: build_iso disk
	$(QEMU) \
		-m $(QEMU_MEMORY) \
		-cdrom "$(ISO_IMAGE)" \
		-drive file="$(DISK_IMAGE)",format=raw,if=virtio \
		-boot d \

		-serial stdio


# =========================================================
# TEST INSTALLED SYSTEM
# =========================================================

test_disk:
	@test -f "$(DISK_IMAGE)" || \
		(echo "ERROR: $(DISK_IMAGE) does not exist. Run: make boot_iso"; exit 1)

	$(QEMU) \
		-m $(QEMU_MEMORY) \
		-drive file="$(DISK_IMAGE)",format=raw,if=virtio \
		-boot c \
		-serial stdio


# =========================================================
# CLEANING
# =========================================================
#
# make clean:
#   removes generated build artifacts but PRESERVES the
#   installed test disk.
#
# make distclean:
#   also removes the virtual disk.
#
# =========================================================

clean: clean_iso clean_programs clean_desktop
	rm -rf "$(INSTALLER_DIR)"
	rm -rf "$(INITRAMFS_DIR)"
	rm -rf "$(DEV_INITRAMFS_DIR)"
	rm -rf "$(PROGRAM_BUILD_DIR)"
	rm -f "$(ISO_IMAGE)"


distclean: clean clean_disk
	@echo "[+] Full clean complete."

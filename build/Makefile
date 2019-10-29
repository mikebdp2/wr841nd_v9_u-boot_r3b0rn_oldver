#ifndef DEV_NAME
#$(error "You have to define Dev name to use this Makefile")
#endif
export DEV_NAME=wr841nv9_en

export TOPDIR=$(PWD)/..
export PRODUCT_CONFIG_DIR=$(TOPDIR)/build/products_config/$(DEV_NAME)/

include $(PRODUCT_CONFIG_DIR)/basic.config

ifndef BOARD_TYPE
export BOARD_TYPE=ap143
endif

ifndef TOOLCHAIN
$(error "Must specify TOOLCHAIN value")
endif

ifndef TOOLARCH
$(error "Must specify TOOLARCH value")
endif

ifndef  BUILD_OPTIMIZED
export BUILD_OPTIMIZED=n
export STRIP=$(TOOLPREFIX)strip
endif

ifeq ($(BUILD_OPTIMIZED), y)
export OLDSTRIP=$(TOOLPREFIX)strip
export STRIP=$(TOOLPREFIX)sstrip
endif

ifndef COMPRESSED_UBOOT
export COMPRESSED_UBOOT=0
endif

ifeq ($(BOOT_FROM_NAND),1)
export NAND=-nand
else
export NAND=
endif

ifneq ($(COMPRESSED_UBOOT),1)
export UBOOTFILE=u-boot.bin
export UBOOT_BINARY=u-boot.bin
else
export UBOOTFILE=tuboot.bin
export UBOOT_BINARY=tuboot.bin
endif
export UBOOT_FILE=u-boot.bin

###
export MAKEARCH=$(MAKE) ARCH=$(KERNELARCH) CROSS_COMPILE=$(TOOLPREFIX)
export TOOLPATH=$(TOPDIR)/build/$(TOOLCHAIN)/$(TOOLARCH)/
export UBOOTDIR=$(TOPDIR)/$(BOARD_TYPE)/boot/u-boot
###

# Save PATH for later use for compiling host-only tools etc.
export ORIGINAL_PATH:=$(PATH)
# Set PATH so we find target compiler when say "gcc", etc.
# as well as other tools we expect to find.
export PATH:=$(TOOLPATH)/bin:${PATH}

all: toolchain_prep uboot_clean uboot

######## prepare toolchain ########
toolchain_prep:
	@echo Uncompressing toolchain, please wait for a few minutes ...
	cd $(TOPDIR)/toolchain/ && \
	rm -rf ./gcc-4.3.3/ && \
	tar -jxf gcc-4.3.3.tar.bz2 && \
	rm -rf $(TOPDIR)/build/gcc-4.3.3 && \
	mv ./gcc-4.3.3/ $(TOPDIR)/build/

######## clean bootloader ########
uboot_clean:
	echo Making uboot clean
	cd $(UBOOTDIR) && $(MAKEARCH) distclean

######## build bootloader ########
uboot:
	@echo Making uboot at `date`
	cd $(UBOOTDIR) && $(MAKEARCH) mrproper
	cd $(UBOOTDIR) && $(MAKEARCH) $(BOARD_TYPE)_config
	@echo ========= build dir: $(TOPDIR)/build ============
	cd $(UBOOTDIR) && $(MAKEARCH) all BUILD_DIR=$(TOPDIR)/build
	@echo End Making uboot at `date`

###

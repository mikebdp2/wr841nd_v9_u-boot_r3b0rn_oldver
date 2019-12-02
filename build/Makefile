#ifndef DEV_NAME
#$(error "You have to define Dev name to use this Makefile")
#endif
export DEV_NAME=wr841nv9_en

export TOPDIR=$(PWD)/..
export PRODUCT_CONFIG_DIR=$(TOPDIR)/build/products_config/$(DEV_NAME)
export BUILD_DIR=$(TOPDIR)/build
### R3B0RN gcc-4.3.3/build_mips => gcc-mips
export STAGING_DIR=$(BUILD_DIR)/gcc-mips/staging_dir
export MACMODELPIN_DIR=$(TOPDIR)/macmodelpin

export UBOOTDIR=$(TOPDIR)/$(BOARD_TYPE)/boot/u-boot
export TUBOOT_DIR=$(TOPDIR)/tuboot

include $(PRODUCT_CONFIG_DIR)/basic.config

ifndef BOARD_TYPE
export BOARD_TYPE=ap143
endif

ifndef TOOLCHAIN
$(error "Must specify TOOLCHAIN value")
endif

ifndef TOOLPREFIX
$(error "Must specify TOOLPREFIX value. For example, make all TOOLPREFIX=mips-linux-uclibc- FLASH_SIZE=4")
endif

ifndef TOOLARCH
$(error "Must specify TOOLARCH value")
endif

ifndef FLASH_SIZE
$(error "Must specify FLASH_SIZE value. For example, make all TOOLPREFIX=mips-linux-uclibc- FLASH_SIZE=4")
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
###

# Save PATH for later use for compiling host-only tools etc.
export ORIGINAL_PATH:=$(PATH)
# Set PATH so we find target compiler when say "gcc", etc.
# as well as other tools we expect to find.
export PATH:=$(TOOLPATH)/bin:${PATH}

### all: toolchain_tplink uboot_clean uboot
all: uboot_clean lzma tuboot_mmp

######## build a good lzma ########
lzma:
	if [ ! -f "$(TOPDIR)/util/lzma/lzma-4.32.7/src/lzma/lzma" ] ; then \
		cd $(TOPDIR)/util/lzma && \
		rm -rf ./lzma-4.32.7/ && tar -zxvf ./lzma-4.32.7.tar.gz && \
		cd ./lzma-4.32.7/ && ./configure && make ; \
	fi

######## prepare toolchain ########
toolchain_tplink:
	@echo Uncompressing toolchain, please wait for a few minutes ...
	cd $(TOPDIR)/toolchain/ && \
	rm -rf ./gcc-4.3.3/ && \
	tar -jxf gcc-4.3.3.tar.bz2 && \
	rm -rf $(BUILD_DIR)/gcc-mips/ && \
	mkdir $(BUILD_DIR)/gcc-mips/ && \
	mv ./gcc-4.3.3/build_mips/staging_dir/ $(BUILD_DIR)/gcc-mips/staging_dir/ && \
	rm -rf ./gcc-4.3.3/

######## clean bootloader ########
uboot_clean:
	echo Making uboot clean
	cd $(UBOOTDIR) && $(MAKEARCH) distclean
	rm -rf $(TUBOOT_DIR) && mkdir $(TUBOOT_DIR)

######## build bootloader ########
uboot:
	@echo Making uboot at `date`
	cd $(UBOOTDIR) && $(MAKEARCH) mrproper
	cd $(UBOOTDIR) && $(MAKEARCH) $(BOARD_TYPE)_config
	@echo ========= build dir: $(BUILD_DIR) ============
	cd $(UBOOTDIR) && $(MAKEARCH) all BUILD_DIR=$(BUILD_DIR)
	@echo End Making uboot at `date`

######## expand bootloader ########
tuboot: uboot
	@echo
	@echo ========= $(TUBOOT_DIR)/tuboot_128kb.bin ============
	rm -rf $(TUBOOT_DIR) && mkdir $(TUBOOT_DIR)
	cp $(UBOOTDIR)/tuboot.bin $(TUBOOT_DIR)/tuboot.bin
	tr '\0' '\377' < /dev/zero | dd bs=1024 count=128 of=$(TUBOOT_DIR)/tuboot_128kb.bin
	dd if=$(TUBOOT_DIR)/tuboot.bin of=$(TUBOOT_DIR)/tuboot_128kb.bin conv=notrunc
	ls -al $(TUBOOT_DIR)/tuboot_128kb.bin

######## build macmodelpin ########
macmodelpin:
	gcc -o $(MACMODELPIN_DIR)/macmodelpin $(MACMODELPIN_DIR)/macmodelpin.c
#=== USAGE ===
#    to print:
#        ./macmodelpin dump.bin
#    to set:
#        ./macmodelpin dump.bin 0xMAC 0xMODEL PIN
#        ./macmodelpin dump.bin 0x16A2594B37DF 0x0841000900000001 12345678

######## setup bootloader ########
tuboot_mmp: tuboot macmodelpin
	cp $(TUBOOT_DIR)/tuboot_128kb.bin $(TUBOOT_DIR)/tuboot_128kb_mmp1.bin
	cp $(TUBOOT_DIR)/tuboot_128kb.bin $(TUBOOT_DIR)/tuboot_128kb_mmp2.bin
	cp $(TUBOOT_DIR)/tuboot_128kb.bin $(TUBOOT_DIR)/tuboot_128kb_mmp3.bin
	cp $(TUBOOT_DIR)/tuboot_128kb.bin $(TUBOOT_DIR)/tuboot_128kb_mmp4.bin
	cp $(TUBOOT_DIR)/tuboot_128kb.bin $(TUBOOT_DIR)/tuboot_128kb_mmp5.bin
	### 1st config
	$(MACMODELPIN_DIR)/macmodelpin $(TUBOOT_DIR)/tuboot_128kb_mmp1.bin \
			0x16A2594B37DF 0x0841000900000001 12345678
	### 2nd config
	$(MACMODELPIN_DIR)/macmodelpin $(TUBOOT_DIR)/tuboot_128kb_mmp2.bin \
			0x16A2594B37DF 0x0841000900000001 12345678
	### 3rd config
	$(MACMODELPIN_DIR)/macmodelpin $(TUBOOT_DIR)/tuboot_128kb_mmp3.bin \
			0x16A2594B37DF 0x0841000900000001 12345678
	### 4th config
	$(MACMODELPIN_DIR)/macmodelpin $(TUBOOT_DIR)/tuboot_128kb_mmp4.bin \
			0x16A2594B37DF 0x0841000900000001 12345678
	### 5th config
	$(MACMODELPIN_DIR)/macmodelpin $(TUBOOT_DIR)/tuboot_128kb_mmp5.bin \
			0x16A2594B37DF 0x0841000900000001 12345678
	### sha256sum
	sha256sum $(TUBOOT_DIR)/* > $(TUBOOT_DIR)/sha256sum.txt
	@echo
	cat $(TUBOOT_DIR)/sha256sum.txt
	@echo

###

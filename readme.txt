		TP-LINK GPL **U-Boot** code + R3B0RN project readme

1. This package still has **only** U-Boot GPL code used by TP-Link TL-WR841ND v9 Router.
TP-Link's ancient 32-bit uClibc gcc-4.3.3 toolchain has been initially required to build,
and one of the goals of R3B0RN project was to slightly modify these sources - just enough
to make them buildable with your own much better opensource toolchain! Search for R3B0RN

	find . -type f -print0 | xargs -0 grep -n "R3B0RN"

and look through a commit history for more information. Maybe later it could be upgraded
to a newer U-Boot version, however it's a low priority. Also, I've copied a macmodelpin
tool from https://github.com/mikebdp2/macmodelpin repository, for setting these values
after the end of U-Boot's compiled binary which has been expanded to 128KB with 0xff .

2. U-Boot build OK on Artix Linux ( https://artixlinux.org/ great fresh no-SystemD Arch )
with multilib ( https://wiki.archlinux.org/index.php/Official_repositories#multilib ) and
32-bit toolchain needs some 32-bit packages: lib32-glibc, lib32-zlib and lib32-gcc-libs.
Your own toolchain perhaps will be 64-bit so multilib/32-bit packages wouldn't be needed.

3. All the other stuff from this package has been removed for simplicity: if you need it,
please get a complete package from https://github.com/mikebdp2/wr841nv9_en_gpl
https://github.com/mikebdp2/wr841n_v9_u-boot has more info about what exactly is removed.

BOARD_TYPE definitions
1. ap143: TL-WR841N/ND 9.0

DEV_NAME definitions
1. wr841nv9_en: TL-WR841N/ND 9.0

Build Instructions

1. All build targets are in ./wr841nd_v9_u-boot_r3b0rn/build/Makefile ,
you should enter this directory to build components:

        cd ./path_to/wr841nd_v9_u-boot_r3b0rn/build/

2. Pre-built 32-bit uClibc gcc-4.3.3 toolchain is available in this package,
however its' source code is available only at toolchain_src of complete package.
If you would like to use it, prepare this toolchain_tplink :

        make TOOLPREFIX=mips-linux-uclibc- FLASH_SIZE=4 toolchain_tplink

But you can get or build your own, much better, opensource toolchain, and install it:

	cd ./path_to/wr841nd_v9_u-boot_r3b0rn/build/
	rm -rf ./gcc-mips/ && mkdir ./gcc-mips/ && mkdir ./gcc-mips/staging_dir/
	cp -R ./path_to/libreCMC/staging_dir/toolchain-mips_24kc_gcc-5.5.0_musl-1.1.16/ \
	                                           ./gcc-mips/staging_dir/usr/
	cp -R ./path_to/libreCMC/staging_dir/host/ ./gcc-mips/staging_dir/host/

Just remember about a TOOLPREFIX : if a gcc of your toolchain is called

	./gcc-mips/staging_dir/usr/bin/mips-openwrt-linux-musl-gcc

then a TOOLPREFIX=mips-openwrt-linux-musl- , in all the subsequent "make" commands.

3. Insert your values to tuboot_mmp of ./path_to/wr841nd_v9_u-boot_r3b0rn/build/Makefile

	nano ./path_to/wr841nd_v9_u-boot_r3b0rn/build/Makefile

4. Build u-boot bootloader with a correct TOOLPREFIX and a FLASH_SIZE in MegaBytes :

        make TOOLPREFIX=mips-openwrt-linux-musl- FLASH_SIZE=4 all

Required U-Boot files will be called " tuboot_128kb_mmp*.bin " and located at

        ls -al ./../tuboot/

5. Overwrite the first 128kb of your router's full chip dump, before flashing to a chip:

        dd if=./../tuboot/tuboot_128kb_mmp1.bin of=./path_to/dump.bin conv=notrunc
        sudo ./path_to/flashrom -p ch341a_spi -c "MX25L3206E/MX25L3208E" -w ./dump.bin -V

P.S. Remember the following memory map of your flash chip:

	1) First 128KB: U-Boot binary expanded with 0xff to 0x20000 and with your values:

	        MAC/MODEL/PIN (0x1FC00,6bytes/0x1FD00,8bytes/0x1FE00,8bytes)

	   If your flash chip size is custom for a particular router, you may need to
	   introduce a new router model value both at MODEL 8bytes/0x1FE00 and firmware :

	        4MB - 0x0841000900000001 or  ||  TPLINK_HWID := 0x08410009
                      0x0841040900000001     ||  TPLINK_HWID := 0x08410409
	        8MB - 0x0841080900000001     ||  TPLINK_HWID := 0x08410809
	       16MB - 0x0841100900000001     ||  TPLINK_HWID := 0x08411009

	   See how the other models in your firmware sources are defined, add a new one.

	2) Middle space: firmware built for this chip size and this router's model

	3) Last 128KB: Atheros Radio Test partition (ART)

			Happy Hacking!

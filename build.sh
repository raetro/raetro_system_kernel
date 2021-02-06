#!/usr/bin/env bash

case "$1" in
	'-u' | '--upload')
		export SSHPASS="1"
		make zImage dtbs
        cat arch/arm/boot/zImage arch/arm/boot/dts/socfpga_cyclone5_chameleon96.dtb > zImage_dtb
		sshpass -e scp zImage_dtb root@192.168.1.52:/media/fat/linux
		sshpass -e ssh root@192.168.1.52 'sync;PATH=/media/fat:$PATH;'
		sshpass -e ssh root@192.168.1.52 'reboot'
		;;
	'-c' | '--clean')
		make clean mrproper 9SiXSTer_defconfig zImage dtbs modules
		;;
	*)
        rm -f zImage_dtb
		make zImage dtbs
        cat arch/arm/boot/zImage arch/arm/boot/dts/socfpga_cyclone5_chameleon96.dtb > zImage_dtb
		;;
esac

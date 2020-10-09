# Rætro Linux Kernel

Linux kernel release 5.x <http://kernel.org/>

## What is Linux?

> Linux is a clone of the operating system Unix, written from scratch by Linus Torvalds with assistance from a loosely-knit team of hackers across the Net. It aims towards POSIX and Single UNIX Specification compliance.

  It has all the features you would expect in a modern fully-fledged Unix, including true multitasking, virtual memory, shared libraries, demand loading, shared copy-on-write executables, proper memory management, and multi-stack networking including IPv4 and IPv6.

## On what hardware does it run?

| Manufacturer  | Hardware    | Platform | Project | Config           | Device Tree Blob                 |
| ------------- | ----------- | -------- | ------- | ---------------- | -------------------------------- |
| Terasic       | DE10-Nano   | ARM/FPGA | MiSTer  | MiSTer_defconfig | socfpga_cyclone5_de10_nano.dtb   |
| Arrow/NovTech | Chameleon96 | ARM/FPGA | Kharma96 | Kharma96_defconfig | socfpga_cyclone5_chameleon96.dtb |

## Requirements to Compile

### On Debian/Ubuntu

```bash
apt-get update
apt-get -y upgrade
apt-get -y install build-essential bc liblz4-tool device-tree-compiler wget libncurses5-dev libncursesw5-dev bison flex libssl-dev
```

### ARM GCC Compiler

```bash
cd /opt
wget -O - https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz | tar xJf -
export ARCH arm
export CROSS_COMPILE /opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
```

## Kernel Download and Configuration

```
wget -O - https://github.com/Raetro/linux_Raetro/archive/master.tar.xz | tar xJf -
```

#### Loading

```bash
make clean mrproper <project_defconfig>
```

#### Configure Kernel (Optional)

```bash
make menuconfig
```

## Compiling the Kernel and Creating zImage with Device Tree Blob

```bash
make zImage modules dtbs
cat arch/arm/boot/zImage arch/arm/boot/dts/<project_device_tree_blob> > zImage_dtb
```

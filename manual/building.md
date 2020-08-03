# Building

## 0. Supported environment

Building skiftOS requires

- A good Linux distro
- nasm
- gcc
- binutils
- grub
- inkscape

And for testing and debugging
- qemu-system-i386
- gdb


```sh
# On Ubuntu
$ sudo apt install nasm gcc make binutils grub-pc-bin qemu-system-x86 xorriso mtools inkscape
```

## 1. Get the source code

Clone the repository with all its submodules.

```sh
$ git clone --recursive https://github.com/skiftOS/skift

$ cd skift
```

## 2. Setting up

Building the toolchain is pretty straight-forward,
first make sure you have all GCC and binutils dependencies:
 - build-essential
 - bison
 - flex
 - libgmp3-dev
 - libmpc-dev
 - libmpfr-dev
 - texinfo

You can run the following command on ubuntu:
```sh
$ sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo
```

Then for building the toolchain run the `build-it.sh` script

```sh
## Build the tool chain
$ toolchain/build-it.sh

## Then wait for completion
```

The script will do the following operation without installing anything to the host system nor requiering root access:
 - Download `gcc` and `binutils` from the GNU project
 - Patch them using binutils.patch and gcc.patch which are located in the toolchain directory.
 - Then configure and build

## 3. Building

From the root of this repo do:

```sh
$ make all
```

This command will build all the component of the project and generate an ISO bootable in QEMU or VirtualBox.

> The compatibility with virtual box is not guaranteed, as we use QEMU primarly for debuging and testing the system

## 4. Running in a virtual machine

The build system allows you to create and start a virtual machine of skiftOS by using one of the following commands:

```sh
$ make run-qemu # for QEMU
# or
$ make run-vbox # for Virtual Box
```

## 5. Tips

> If you made any modification to the source code or the content of the sysroot/ directory, the build system should be able to rebuild the project from step 3 automagically :^)

> You can change the default keyboard layout by passing CONFIG_KEYBOARD_LAYOUT="fr_fr" to make.

## 6. Using the system

**How to change the keyboard layout?**

```sh
µ keyboardctl en_us
```

**How to change display resolution?**

```sh
µ displayctl -s 1920x1080
```
**How to change to wallpaper?**

```sh
µ wallpaperctl /System/Wallpapers/paint.png
```

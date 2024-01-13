# System/28
A hobbyist project aiming to create a new workstation operating system with a kernel written completely from scratch.

![Screenshot](https://i.imgur.com/XZyDAm9.png)

## 📃 Current features:
- SMP/multiprocessing support
- Exception handling
- Interrupt support, including timer and keyboard IRQs
- Virtual memory (paging) support
- Physical memory management & heap manager
- Ramdisk support
- Local and I/O APICs in place of 8259 PIC
- Basic ACPI support (for APICs, reboot, and power info)

## 🔨 Build instructions:
To build System/28, MacOS or Linux systems are recommended.

**1. Install dependencies**
- For MacOS (Homebrew):
    ```
    $ brew install nasm xorriso x86_64-elf-gcc qemu
    ```
- For Ubuntu:
    ```
    $ sudo apt install nasm xorriso qemu-system-x86
    ```
    Follow guide at [OSDev Wiki](https://wiki.osdev.org/GCC_Cross-Compiler) in order to build x86_64-elf cross toolchain.

**2. Clone repo**
```
$ git clone https://github.com/danthedev123/system28
$ cd system28
```
**2. Build & test System/28**
```
$ ./bootstrap.sh
$ make run
```

## 🌎 External software used:
- [Limine bootloader & protocol](https://github.com/limine-bootloader/limine)
- [Flanterm terminal emulator](https://github.com/mintsuki/flanterm)

## 🕰️ History
System/28 is a rewrite of [System/14](https://www.github.com/danthedev123/system14_archive), a project I started working on in May 2023. Before that, I had always been interested in OSDev and had also made some crappy projects that could barely do anything. System/14 is the first project that could do scheduling or any real OS functions. I decided to rewrite it for code quality reasons.

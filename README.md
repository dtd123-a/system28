<h1 align="center">System/28</h2>
A hobby programming project aiming to create a new workstation operating system with a kernel written completely from scratch. The kernel is written in C++, and some C and Assembly.

![Screenshot of System/28 kernel monitor](https://i.imgur.com/XZyDAm9.png)

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
To build `System/28`, UNIX-like systems are recommended.

**1. Install dependencies**
- For macOS with Homebrew:
    - nasm/xorriso/qemu:
    ```
    $ brew install nasm xorriso qemu
    ```
    - Cross compilation toolchain:
    ```
    $ brew install x86_64-elf-gcc
    ```
- Linux only:
    Example command for Ubuntu/Debian/Mint (not including the cross compiler):
    ```
    $ sudo apt install nasm xorriso qemu-system-x86
    ```
    - Adjust this command for your distribution's package manager.
    - If your package manager doesn't ship `x86_64-elf-gcc`, such as in the case of Ubuntu and Debian, follow guide at [OSDev Wiki](https://wiki.osdev.org/GCC_Cross-Compiler) or use Homebrew on Linux **(see Mac instructions above)** in order to install the toolchain.

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

## 🏅 Miletones:
- Sep 2, 2023: Physical page frame allocator implemented
- Sep 2, 2023: SMP working
- Sep 8, 2023: Virtual memory support working
- Sep 16, 2023: Heap manager added
- Sep 26, 2023: ACPI reboot working on real hardware
- Oct 1, 2023: Ramdisk manager implemented
- Apr 11, 2024: Milestones added to readme

## 🕰️ History
- First commit at September 1st, 2023.
- System/28 is a rewrite of [System/14](https://www.github.com/danthedev123/system14_archive), a project I started working on in May 2023.
- System/14 is my first OSDev attempt that got as far as scheduling with processes, with fully original code.
- The rewrite, System/28, is for codebase quality and maintenance reasons.

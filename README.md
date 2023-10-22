# 💻 System/28
A project aiming to create a new workstation operating system with a kernel written completely from scratch.

## 📃 Current features:
- SMP/multiprocessing support
- Exception handling
- Interrupt support, including timer and keyboard IRQs
- Virtual memory (paging) support
- Physical memory management & heap manager
- Ramdisk support
- Local and I/O APICs in place of 8259 PIC
- Basic ACPI support (for APICs, reboot, and power info)

## 🕰️ History
System/28 is a rewrite of [https://github.com/danthedev123/system14_archive](System/14), a project I started working on in May 2023. Before that, I had always been interested in OSDev and had also made some crappy projects that could barely do anything. System/14 is the first project that could do scheduling or any real OS functions. I decided to rewrite it for code quality reasons.

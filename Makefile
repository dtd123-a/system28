rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

cpp := clang++
cc := clang
ld := ld
kbin := kernel.elf

C_CPP_COMMONFLAGS += \
	-target x86_64-elf \
    -Wall \
    -Wextra \
    -Werror \
	-Wno-interrupt-service-routine \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIE \
    -fno-PIC \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel \
    -Ikernel/include \
	-g \
	-Og \

CPPFLAGS += \
	-fno-exceptions \
	-fno-rtti \
	-fno-use-cxa-atexit \
	-std=c++17

LDFLAGS += \
    -nostdlib \
    -static \
    -m elf_x86_64 \
    -z max-page-size=0x1000 \
    -T kernel/misc/kernel.ld

ASMFLAGS += \
    -Wall \
    -f elf64

QEMUFLAGS += -smp 4 -m 1G -cdrom root.iso -d int -D qemu.log
UEFIFW += --bios /usr/share/ovmf/OVMF.fd

kcsources = $(call rwildcard,kernel/src,*.cpp)
kcsources_c = $(call rwildcard,kernel/src,*.c)
kcsourcesnasm = $(call rwildcard,kernel/src,*.asm)
kobj = $(kcsourcesnasm:.asm=.o) $(kcsources:.cpp=.o) $(kcsources_c:.c=.o)

all: $(kbin) limine

limine:
	@git clone https://github.com/limine-bootloader/limine.git --branch=v6.x-branch-binary --depth=1
	@make -C limine

$(kbin): $(kobj)
	@echo 'LD ' $(kbin)
	@$(ld) $(kobj) $(LDFLAGS) -o $@

kernel/src/%.o: kernel/src/%.cpp
	@echo CXX $<
	@$(cpp) $(C_CPP_COMMONFLAGS) $(CPPFLAGS) -c $< -o $@

kernel/src/%.o: kernel/src/%.c
	@echo 'CC ' $<
	@$(cc) $(C_CPP_COMMONFLAGS) -c $< -o $@

kernel/src/%.o: kernel/src/%.asm
	@echo 'AS ' $<
	@nasm $(ASMFLAGS) $< -o $@

iso: $(kbin)
	@rm -rf iso_root
	@mkdir -p iso_root
	@cp kernel.elf ramdisk.tar kernel/misc/limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/
	@xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o root.iso
	@limine/limine bios-install root.iso
	@rm -rf iso_root

run: iso
	qemu-system-x86_64 $(QEMUFLAGS) -nographic

run_graphic: iso
	qemu-system-x86_64 $(QEMUFLAGS)

run_uefi: iso
	qemu-system-x86_64 $(QEMUFLAGS) $(UEFIFW)

rungdb: iso
	qemu-system-x86_64 $(QEMUFLAGS) -s -S

rungdb_uefi: iso
	qemu-system-x86_64 $(QEMUFLAGS) $(UEFIFW) -s -S

clean:
	-@rm $(kobj)
	-@rm -f kernel.elf

bootstrap:
	@bash bootstrap.sh

unbootstrap:
	@rm -rf kernel/src/external_libs/flanterm

.PHONY: ramdisk
ramdisk:
	@tar cvf ramdisk.tar ramdisk/

/*
    * mod.cpp
    * Handles bootloader modules given to the kernel.
    * Created 01/10/2023
*/
#include <obj/mod.hpp>
#include <libs/kernel.hpp>
#include <early/bootloader_data.hpp>
#include <hal/vmm.hpp>
#include <obj/tar.hpp>

extern BootloaderData GlobalBootloaderData;

namespace Kernel::Obj {
    struct InternalModule {
        void *VirtualAddress;
        const char *Path;
        const char *CommandLine;
        size_t MediaType;
    };

    Lib::Vector<InternalModule> *Modules;
    TarObject *FirstRamdisk;

    void HandleModuleObjects(limine_module_response moduleStructure) {
        Modules = new Lib::Vector<InternalModule>();

        for (size_t i = 0; i < moduleStructure.module_count; i++) {
            uintptr_t virtAddr = (uintptr_t)moduleStructure.modules[i]->address;
            uintptr_t physAddr = virtAddr - GlobalBootloaderData.hhdm_response.offset;

            VMM::MemoryMap(nullptr, virtAddr, physAddr, false);

            Modules->push_back(InternalModule {
                .VirtualAddress = (void *)virtAddr,
                .Path = moduleStructure.modules[i]->path,
                .CommandLine = moduleStructure.modules[i]->cmdline,
                .MediaType = moduleStructure.modules[i]->media_type
            });
        }

        /* Here, the first module (if it exists) is initialized as a ramdisk. */
        /* Note: This is only temporary. Once we have a VFS and multiple ramdisks can be handled, all ramdisk modules will be set up. */
        if (Modules->at(0).VirtualAddress) {
            FirstRamdisk = new TarObject(Modules->at(0).VirtualAddress);
        }
    }
}
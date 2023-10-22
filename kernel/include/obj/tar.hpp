/*
    * tar.hpp
    * Tape Archive (TAR) ramdisk class definitions
    * Created 30/09/2023
*/
#pragma once
#include <stddef.h>
#include <libs/kernel.hpp>

namespace Kernel::Obj {
    class TarObject {
    public:
        struct File {
            /* Full file path (e.g /file.txt)*/
            const char *Path;
            /* Links to (hard links) */
            File *LinksTo;
            /* Size of the file (bytes) */
            size_t FileSize;
            /* Type flag */
            char Type;
            /* The location in memory where the ramdisk file's contents is */
            void *FileAddress;
        };

        TarObject(void *RamdiskPtr);
        ~TarObject();
        File Get(const char *Path);
        Lib::Vector<File> GetAll();
private:
        Lib::Vector<File> Files;
    };
}

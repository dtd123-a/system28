/*
    * tar.hpp
    * Tape Archive (TAR) ramdisk class definitions
    * Created 30/09/2023
*/
#include <stddef.h>
#include <libs/kernel.hpp>

namespace Kernel::Obj {
    class TarObject {
    public:
        struct File {
            /* Full file path (e.g A:/file.txt)*/
            const char *Path;
            /* Size of the file (bytes) */
            size_t FileSize;
            /* The location in memory where the ramdisk file's contents is */
            void *FileAddress;
        };

        TarObject(void *RamdiskPtr);
        File Get(const char *Path);
        Lib::Vector<File> GetAll();
private:
        Lib::Vector<File> Files;
    };
}

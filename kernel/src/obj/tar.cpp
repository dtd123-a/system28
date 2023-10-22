/*
    * tar.cpp
    * Unix Standard Tape Archive (USTAR) driver implementation
    * Created 30/09/2023
*/

#include <obj/tar.hpp>
#include <libs/string.hpp>
#include <mm/mem.hpp>
#include <terminal/terminal.hpp>

/* USTAR is the standard extended version of the TAR file system. */
struct USTARHeader {
    char FileName[100];
    char Mode[8];
    char UID[8];
    char GID[8];
    char Size[12];
    char MTime[12];
    char Checksum[8];
    char Typeflag;
    char Linkname[100];
    /* The magic can be used to verify we have an actual USTAR archive. Contains the string "ustar\0" */
    char Magic[6];
    char Version[2];
    char UserName[32];
    char GroupName[32];
    char DevMajor[8];
    char DevMinor[8];
    char Prefix[155];
};

/* Decodes USTARs's 'octal string' method for encoding number data. */
size_t DecodeTarNumeral(char value[]) {
    size_t ret = 0;
    size_t count = 1;

    for (int j = 11; j > 0; j--, count *= 8) {
        ret += ((value[j - 1] - '0') * count);
    }

    return ret;
}

namespace Kernel::Obj {
    TarObject::TarObject(void *RamdiskPtr) {
        USTARHeader *Header = (USTARHeader *)RamdiskPtr;
        
        /* Verify the USTAR magic */
        if (strncmp(Header->Magic, "ustar", 5)) {
            /* The verification has failed.*/
            return;
        }

        while (*(uint8_t *)Header != 0) {
            /* Remove any trailing slashes at the end of directory names. */
            /* Before USTAR type attribute this was how directories were recorded. */
            int len = strlen(Header->FileName);
            if (Header->FileName[len - 1] == '/') {
                Header->FileName[len - 1] = '\0';
            }

            Files.push_back(File {
                .Path = (const char *)Header->FileName,
                .LinksTo = nullptr,
                .FileSize = DecodeTarNumeral(Header->Size),
                .Type = Header->Typeflag,
                .FileAddress = (uint8_t *)Header + 512
            });

            Header = (USTARHeader *)((uint8_t *)Header + ALIGN_UP(DecodeTarNumeral(Header->Size) + 512, 512));
        }
    }

    TarObject::~TarObject() {
        Files.~Vector();
    }

    TarObject::File TarObject::Get(const char *Path) {
        for (size_t i = 0; i < Files.size(); i++) {
            if (strncmp(Files.at(i).Path, Path, 100) == 0) {
                return Files.at(i);
            }
        }

        return File {0, 0, 0, 0, 0};
    }

    Lib::Vector<TarObject::File> TarObject::GetAll() {
        return Files;
    }
}

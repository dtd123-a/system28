/*
    * smp.cpp
    * Symmetric multi-processing
    * Created 02/09/2023
*/

#include <limine.h>

limine_smp_info* smp_info = nullptr;

namespace Kernel::CPU {
    void SMPSetup(limine_smp_info* smp_info)
    {
        ::smp_info = smp_info;
    }

    void CPUJump(unsigned int cpu, void* ptr)
    {
        if (!cpu) return; // CPU must be > 0
        if (!smp_info) return;

        smp_info[cpu].goto_address = (limine_goto_address)ptr;
    }
}
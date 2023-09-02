/*
    * smp.hpp
    * Header for symmetric multi-processing
    * Created 02/09/2023
*/
#pragma once
#include <limine.h>

namespace Kernel::CPU {
    void SMPSetup(limine_smp_info* smp_info);
    void CPUJump(unsigned int cpu, void* ptr);
}
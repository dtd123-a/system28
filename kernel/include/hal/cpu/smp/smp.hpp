/*
    * smp.hpp
    * Header for symmetric multi-processing
    * Created 02/09/2023
*/
#pragma once
#include <limine.h>

namespace Kernel::CPU {
    void CPUJump(unsigned int cpu, void* ptr);
    void CPUShutdown();
    void SetupAllCPUs();
}
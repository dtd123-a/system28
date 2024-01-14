/*
    * smp.hpp
    * Header for symmetric multi-processing
    * Created 02/09/2023
*/
#pragma once
#include <limine.h>
#include <stdint.h>

namespace Kernel::CPU {
    void CPUJump(uint32_t Core, void* Target);
    void SetupAllCPUs();
}

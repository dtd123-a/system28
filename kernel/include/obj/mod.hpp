/*
    * mod.hpp
    * Handles bootloader modules given to the kernel.
    * Created 01/10/2023
*/
#pragma once
#include <limine.h>
#include <stddef.h>

namespace Kernel::Obj {
    void HandleModuleObjects(limine_module_response *moduleStructure);
}

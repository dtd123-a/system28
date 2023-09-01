/*
    * terminal.hpp
    * Declares terminal functions
    * Created 01/09/23 DanielH
*/

#pragma once
#include <stdint.h>

namespace Kernel {
    namespace Init {
        void InitializeFlanterm(uint32_t *framebuffer, int width, int height, int pitch);
    }

    namespace Lib {
        void Print(const char* string);
    }
}
/*
    * terminal.cpp
    * Implements terminal functions
    * Created 01/09/23 DanielH
*/

#include <stdint.h>
#include <terminal/terminal.hpp>
#include <external_libs/flanterm/flanterm.h>
#include <external_libs/flanterm/backends/fb.h>

flanterm_context* fb_ctx = nullptr;

namespace Kernel {
    namespace Init {
        void InitializeFlanterm(uint32_t *framebuffer, int width, int height, int pitch) {
            fb_ctx = flanterm_fb_simple_init(framebuffer, width, height, pitch);
        }
    }
}

void PutChar(char c)
{
    flanterm_write(fb_ctx, &c, 1);
}

namespace Kernel {
    namespace Lib {
        void Print(const char *string) {
            while (*string != '\0') {
                PutChar(*string);
                string++;
            }
        }
    }
}

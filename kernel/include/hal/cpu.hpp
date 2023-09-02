/*
    * cpu.hpp
    * Declares/implements CPU abstractions
    * Created 01/09/23 DanielH
*/

namespace Kernel {
    namespace CPU {
        inline void NoOp() { asm volatile ("nop"); }
        inline void Halt() { asm volatile ("hlt"); }
        inline void ClearInterrupts() { asm volatile ("cli"); }
        inline void SetInterrupts() { asm volatile ("sti"); }

        void Initialize();
    }
}

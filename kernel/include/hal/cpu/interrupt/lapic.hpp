
/*
    * lapic.hpp
    * Implements the local APIC and its timer.
    * Created 11/09/23 DanielH
*/
#pragma once

namespace Kernel::CPU {
    void InitializeLAPIC();
    void LAPIC_EOI();
    void TimerReset();
}
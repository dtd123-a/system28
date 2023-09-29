# System/28
A rewrite of System/14 meant to be half as crappy. This time I am using C, C++ and Assembly. The aim is for it to reimplement all of System/14's features, (but less crappy), then work on new things.

# Progress
- [x] Basic CPU HAL (GDT, CPU functions, etc)
- [x] PMM (take it from System/14, less crappy due to refactor?)
- [x] Interrupts
- [x] SMP (Symmetric Multi-Processing)
- [x] Virtual memory (paging)
- [x] LAPIC and I/O APIC
- [x] APIC timer calibration via PMT
- [x] Heap allocation
- [x] Timer
- [ ] Scheduling (again)

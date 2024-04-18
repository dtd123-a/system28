/*
    * serial.h
    * Serial output/debugging implementation for System/28 kernel
    * Created 18/04/2024
*/
#pragma once
#include <stdint.h>
#include <libs/kernel.hpp>

namespace Kernel::Debug {
    /* Serial port constants (COM1-COM8) */
    enum SerialPortValue {
        NoPort = 0x000,
        COM1 = 0x3F8,
        COM2 = 0x2F8,
        COM3 = 0x3E8,
        COM4 = 0x2E8,
        COM5 = 0x5F8,
        COM6 = 0x4F8,
        COM7 = 0x5E8,
        COM8 = 0x4E8
    };

    class SerialPort {
        SerialPortValue PortID = COM1;

        bool ProgramSerial();

        public:
        SerialPort(SerialPortValue Port);
        SerialPort();
        void BlockUntilEmpty();
        void WriteCharacter(char character);
        void WriteString(const char *string);
    };
}
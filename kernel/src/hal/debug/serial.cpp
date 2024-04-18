/*
    * serial.cpp
    * Serial output/debugging implementation for System/28 kernel
    * Created 18/04/2024
*/
#include <hal/debug/serial.hpp>

using namespace Kernel::Debug;

bool SerialPort::ProgramSerial() {
    if (PortID == NoPort) return false;

    /* Prepare the port for usage */
    IO::outb(PortID + 1, 0x00); // Disable interrupts
    IO::outb(PortID + 3, 0x80); // No idea what this does.
    IO::outb(PortID + 0, 0x03); // Set divisor
    IO::outb(PortID + 1, 0x00);
    IO::outb(PortID + 3, 0x03);
    IO::outb(PortID + 2, 0xC7);
    IO::outb(PortID + 4, 0x0B);
    IO::outb(PortID + 4, 0x0F); // Operational mode

    return true;
}

SerialPort::SerialPort(SerialPortValue Port) {
    PortID = Port;
    ProgramSerial();
}

SerialPort::SerialPort() {
    ProgramSerial();
}

void SerialPort::BlockUntilEmpty() {
    while (!(IO::inb(PortID + 5) & 0x20));
}

void SerialPort::WriteCharacter(char character) {
    BlockUntilEmpty();
    IO::outb(PortID, character);
}

void SerialPort::WriteString(const char *string) {
    while (*string != '\0') {
        WriteCharacter(*string);
        string++;
    }
}
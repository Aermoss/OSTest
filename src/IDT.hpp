#pragma once

#include <stdint.h>

#include "IO.hpp"

struct InterruptDesc {
    uint16_t offsetLow;
    uint16_t selector;
    uint8_t ist;
    uint8_t typeAttr;
    uint16_t offsetMid;
    uint32_t offsetHigh;
    uint32_t zero;
};

extern InterruptDesc IDT[256];
extern uint64_t ISR;

extern "C" void LoadIDT();

void InitializeIDT() {
    IDT[1] = {
        .offsetLow = (uint16_t) ((uint64_t) &ISR & 0x000000000000FFFF),
        .selector = 0x08,
        .ist = 0,
        .typeAttr = 0x8E,
        .offsetMid = (uint16_t) (((uint64_t) &ISR & 0x00000000FFFF0000) >> 16),
        .offsetHigh = (uint32_t) (((uint64_t) &ISR & 0xffffffff00000000) >> 32),
        .zero = 0
    };

    RemapPIC();
    outb(0x21, 0xfd);
    outb(0xa1, 0xff);
    LoadIDT();
}

void (*keyboardHandler)(uint8_t scancode);

extern "C" void ISRHandler() {
    uint8_t scancode = inb(0x60);

    if (keyboardHandler != nullptr)
        keyboardHandler(scancode);

    outb(0x20, 0x20);
    outb(0xa0, 0x20);
}
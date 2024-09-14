#include <stdint.h>
#include <limits.h>

#define VIDEO_MEMORY 0xb8000
#define ROW_SIZE 80
#define COL_SIZE 25

char* IntToString(int value, char* str, int base) {
    char *rc, *ptr, *low;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    rc = ptr = str;
    if (value < 0 && base == 10) *ptr++ = '-';
    low = ptr;

    do {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + value % base];
        value /= base;
    } while (value);

    *ptr-- = '\0';

    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    } return rc;
}

uint8_t videoBuffer[ROW_SIZE * COL_SIZE * 2];
uint64_t cursorPos = 0;

void EnterPanicMode(const char* reason);

void SetCharAt(uint64_t pos, uint8_t chr, uint8_t color = 0x00 | 0x0F) {
    if (pos >= ROW_SIZE * COL_SIZE)
        EnterPanicMode("Cursor position out of bounds.");

    videoBuffer[pos * 2] = chr;
    videoBuffer[pos * 2 + 1] = color;
}

void WriteString(const char* string, uint8_t color = 0x00 | 0x0F) {
    for (uint64_t i = 0; string[i] != '\0'; i++) {
        if (string[i] == '\n') {
            cursorPos += ROW_SIZE - (cursorPos % ROW_SIZE);
        } else {
            SetCharAt(cursorPos++, string[i], color);
        }
    }
}

void Clear(uint8_t color = 0x00 | 0x00) {
    for (uint64_t i = 0; i < ROW_SIZE * COL_SIZE; i++) {
        SetCharAt(i, 219, color);
    } cursorPos = 0;
}

void SwapBuffers() {
    for (uint64_t i = 0; i < ROW_SIZE * COL_SIZE * 2; i++)
        *(uint8_t*) (VIDEO_MEMORY + i) = videoBuffer[i];
}

void EnterPanicMode(const char* reason) {
    while (true) {
        Clear(0x00 | 0x04);
        WriteString("Kernel panic occured, system halted.\n", 0x00 | 0x04);
        WriteString("Reason: ", 0x00 | 0x04);
        WriteString(reason, 0x00 | 0x04);
        SwapBuffers();
    }
}

extern "C" void _start() {
    uint64_t x = 0;

    while (true) {
        Clear();
        WriteString("Hello, World!\nTicks: ");
        WriteString(IntToString(x++, nullptr, 10));
        WriteString("\n");

        for (uint64_t i = 0; i < (uint64_t) (x / 100); i++)
            WriteString("Wow!\n"); // This is just to test the panic mode.

        SwapBuffers();
    }
}
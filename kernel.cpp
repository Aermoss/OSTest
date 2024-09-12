#include <stdint.h>
#include <limits.h>

#define VIDEO_MEMORY 0xb8000
#define ROW_SIZE 80
#define COL_SIZE 25

char* itoa(int value, char* str, int base) {
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

uint64_t cursorPos = 0;

void print(const char* message, uint8_t color = 0x00 | 0x0F) {
    for (uint64_t i = 0; 1; i++) {
        if (message[i] == '\0') return;
        else if (message[i] == '\n') {
            cursorPos += ROW_SIZE - (cursorPos % ROW_SIZE);
            continue;
        }

        *(uint8_t*) (VIDEO_MEMORY + cursorPos * 2) = message[i];
        *(uint8_t*) (VIDEO_MEMORY + (cursorPos * 2) + 1) = color;
        cursorPos++;
    }
}

void clear(uint8_t color = 0x00 | 0x00) {
    cursorPos = 0;

    for (uint64_t i = 0; i < ROW_SIZE * COL_SIZE; i++) {
        *(uint8_t*) (VIDEO_MEMORY + i * 2) = 219;
        *(uint8_t*) (VIDEO_MEMORY + (i * 2) + 1) = color;
    }
}

void wait(uint64_t ticks) {
    uint64_t start = 0;
    while (start < ticks) start++;
}

extern "C" void _start() {
    int x = 0;

    while (true) {
        clear();
        print("Hello, World!\nTicks: ");
        print(itoa(x++, nullptr, 10));
        wait(100000000);
    } return;
}
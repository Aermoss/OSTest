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
    }

    return rc;
}

uint64_t cursor_position = 0;
void print(const char* message, uint8_t color = 0x00 | 0x0F) {
    for (uint64_t i = 0; 1; i++) {
        if (message[i] == '\0') return;
        else if (message[i] == '\n') {
            cursor_position += ROW_SIZE - (cursor_position % ROW_SIZE);
            continue;
        }

        *(uint8_t*) (VIDEO_MEMORY + cursor_position * 2) = message[i];
        *(uint8_t*) (VIDEO_MEMORY + (cursor_position * 2) + 1) = color;
        cursor_position++;
    }
}

void clear(uint8_t color = 0x00 | 0x00) {
    for (uint64_t i = 0; i < ROW_SIZE * COL_SIZE; i++) {
        *(uint8_t*) (VIDEO_MEMORY + i * 2) = 219;
        *(uint8_t*) (VIDEO_MEMORY + (i * 2) + 1) = color;
    }
}

extern "C" void _start() {
    print("sgsdg\n");
    print(itoa(199, nullptr, 10));
    return;
}
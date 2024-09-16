#include <stdint.h>
#include <limits.h>

#define VIDEO_MEMORY 0xb8000
#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25

#define FOREGROUND_BLACK 0x00
#define FOREGROUND_BLUE 0x01
#define FOREGROUND_GREEN 0x02
#define FOREGROUND_CYAN 0x03
#define FOREGROUND_RED 0x04
#define FOREGROUND_MAGENTA 0x05
#define FOREGROUND_BROWN 0x06
#define FOREGROUND_LIGHT_GREY 0x07
#define FOREGROUND_DARK_GREY 0x08
#define FOREGROUND_LIGHT_BLUE 0x09
#define FOREGROUND_LIGHT_GREEN 0x0A
#define FOREGROUND_LIGHT_CYAN 0x0B
#define FOREGROUND_LIGHT_RED 0x0C
#define FOREGROUND_LIGHT_MAGENTA 0x0D
#define FOREGROUND_LIGHT_BROWN 0x0E
#define FOREGROUND_WHITE 0x0F

#define BACKGROUND_BLACK 0x00
#define BACKGROUND_BLUE 0x10
#define BACKGROUND_GREEN 0x20
#define BACKGROUND_CYAN 0x30
#define BACKGROUND_RED 0x40
#define BACKGROUND_MAGENTA 0x50
#define BACKGROUND_BROWN 0x60
#define BACKGROUND_LIGHT_GREY 0x70
#define BACKGROUND_DARK_GREY 0x80
#define BACKGROUND_LIGHT_BLUE 0x90
#define BACKGROUND_LIGHT_GREEN 0xA0
#define BACKGROUND_LIGHT_CYAN 0xB0
#define BACKGROUND_LIGHT_RED 0xC0
#define BACKGROUND_LIGHT_MAGENTA 0xD0
#define BACKGROUND_LIGHT_BROWN 0xE0
#define BACKGROUND_WHITE 0xF0

#include "IO.hpp"
#include "IDT.hpp"
#include "Scancode.hpp"

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

uint8_t videoBuffer[VIDEO_WIDTH * VIDEO_HEIGHT * 2];
uint16_t cursorPos = 0;

void EnterPanicMode(const char* reason);

uint16_t PositionFromCoords(uint8_t x, uint8_t y) {
    return y * VIDEO_WIDTH + x;
}

void SetCursorPosition(uint16_t pos) {
    if (pos >= VIDEO_WIDTH * VIDEO_HEIGHT)
        EnterPanicMode("Cursor position out of bounds.");

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void SetCharAt(uint16_t pos, uint8_t chr, uint8_t color = FOREGROUND_WHITE | BACKGROUND_BLACK) {
    if (pos >= VIDEO_WIDTH * VIDEO_HEIGHT)
        EnterPanicMode("Cursor position out of bounds.");

    videoBuffer[pos * 2] = chr;
    videoBuffer[pos * 2 + 1] = color;
}

void WriteString(const char* string, uint8_t color = FOREGROUND_WHITE | BACKGROUND_BLACK) {
    for (uint64_t i = 0; string[i] != '\0'; i++) {
        if (string[i] == '\n') {
            // cursorPos += VIDEO_WIDTH;
            cursorPos += VIDEO_WIDTH - (cursorPos % VIDEO_WIDTH);
        } else if (string[i] == '\r') {
            cursorPos -= cursorPos % VIDEO_WIDTH;
        } else {
            SetCharAt(cursorPos++, string[i], color);
        }
    }
}

void Clear(uint8_t color = FOREGROUND_WHITE | BACKGROUND_BLACK) {
    for (uint16_t i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
        SetCharAt(i, ' ', color);
    } cursorPos = 0;
}

void SwapBuffers() {
    for (uint16_t i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT * 2; i++) {
        *(uint8_t*) (VIDEO_MEMORY + i) = videoBuffer[i];
    } SetCursorPosition(cursorPos);
}

void EnterPanicMode(const char* reason) {
    while (true) {
        Clear(FOREGROUND_BLACK | BACKGROUND_RED);
        WriteString("Kernel panic occured, system halted.\n", FOREGROUND_RED | BACKGROUND_BLACK);
        WriteString("Reason: ", FOREGROUND_RED | BACKGROUND_BLACK);
        WriteString(reason, FOREGROUND_RED | BACKGROUND_BLACK);
        SwapBuffers();
    }
}

extern const char Text[];
extern uint64_t Text_Size;

#define MAX_INPUT_LENGTH 256
#define MAX_INPUT_TEXT_COUNT 8

uint8_t inputText[MAX_INPUT_LENGTH] = { '\0' };
uint8_t enteredTexts[MAX_INPUT_TEXT_COUNT][MAX_INPUT_LENGTH];

bool leftShiftPressed = false;
bool rightShiftPressed = false;
bool capsLockState = false;

void KeyboardHandler(uint8_t scancode) {
    if (scancode < 0x3A && ScancodeLookupTable[scancode] != '\0') {
        for (uint32_t i = 0; i + 1 < MAX_INPUT_LENGTH; i++) {
            if (inputText[i] == '\0') {
                inputText[i] = ScancodeLookupTable[scancode];
                inputText[i + 1] = '\0';

                if ((leftShiftPressed | rightShiftPressed) != capsLockState) {
                    if (inputText[i] >= 'a' && inputText[i] <= 'z')
                        inputText[i] -= 32;
                } break;
            }
        }
    } else {
        switch (scancode) {
            case 0x0E: {
                for (uint32_t i = 0; i < MAX_INPUT_LENGTH; i++) {
                    if (inputText[i] == '\0' && i > 0) {
                        inputText[i - 1] = '\0';
                        break;
                    }
                } break;
            } case 0x2A: {
                leftShiftPressed = true;
                break;
            } case 0xAA: {
                leftShiftPressed = false;
                break;
            } case 0x36: {
                rightShiftPressed = true;
                break;
            } case 0xB6: {
                rightShiftPressed = false;
                break;
            } case 0x3A: {
                capsLockState = !capsLockState;
                break;
            } case 0x9C: {
                if (inputText[0] == '\0')
                    break;

                for (uint32_t i = 1; i < MAX_INPUT_TEXT_COUNT; i++) {
                    for (uint32_t j = 0; j < MAX_INPUT_LENGTH; j++) {
                        enteredTexts[i - 1][j] = enteredTexts[i][j];
                        enteredTexts[i][j] = '\0';
                    }
                }

                for (uint32_t i = 0; i < MAX_INPUT_LENGTH; i++) {
                    enteredTexts[MAX_INPUT_TEXT_COUNT - 1][i] = inputText[i];
                    inputText[i] = '\0';
                } break;
            } default: {
                break;
            }
        }
    }
}

extern "C" void Entry() {
    uint64_t x = 0;
    InitializeIDT();
    keyboardHandler = KeyboardHandler;

    while (true) {
        Clear();
        WriteString(Text);
        WriteString("\nTicks: ");
        WriteString(IntToString(x++, nullptr, 10));
        WriteString("\n");
        
        for (uint64_t i = 0; i < MAX_INPUT_TEXT_COUNT; i++) {
            if (enteredTexts[i][0] == '\0') continue;
            WriteString((const char*) enteredTexts[i]);
            WriteString("\n");
        }
        
        WriteString("> ");
        WriteString((const char*) inputText);
        SwapBuffers();
    }
}
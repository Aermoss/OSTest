#include <stdint.h>
#include <limits.h>

#define DOUBLE_BUFFERING 1

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
uint64_t cursorPos = 0;

void EnterPanicMode(const char* reason);

void SetCursorPosition(uint64_t pos) {
    if (pos >= VIDEO_WIDTH * VIDEO_HEIGHT)
        EnterPanicMode("Cursor position out of bounds.");

#if DOUBLE_BUFFERING
    cursorPos = pos;
#else
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
#endif
    return;
}

uint64_t GetCursorPosition() {
    return cursorPos;
}

uint64_t PositionFromCoords(uint8_t x, uint8_t y) {
    return y * VIDEO_WIDTH + x;
}

void WriteString(const char* string, uint8_t color = FOREGROUND_WHITE | BACKGROUND_BLACK) {
    for (uint64_t i = 0; string[i] != '\0'; i++) {
        if (string[i] == '\n') {
            cursorPos += VIDEO_WIDTH - (cursorPos % VIDEO_WIDTH);
        } else if (string[i] == '\r') {
            cursorPos -= cursorPos % VIDEO_WIDTH;
        } else {
#if DOUBLE_BUFFERING
            videoBuffer[cursorPos * 2] = string[i];
            videoBuffer[cursorPos * 2 + 1] = color;
#else
            *(uint8_t*) (VIDEO_MEMORY + cursorPos * 2) = string[i];
            *(uint8_t*) (VIDEO_MEMORY + cursorPos * 2 + 1) = color;
#endif
            cursorPos++;
        }
    }

    if (cursorPos >= VIDEO_WIDTH * VIDEO_HEIGHT) {
        EnterPanicMode("Cursor position out of bounds.");
    } return;
}

void Clear(uint8_t color = FOREGROUND_WHITE | BACKGROUND_BLACK) {
    for (uint64_t i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
#if DOUBLE_BUFFERING
        videoBuffer[i * 2] = ' ', videoBuffer[i * 2 + 1] = color;
#else
        *(uint8_t*) (VIDEO_MEMORY + i * 2) = ' ';
        *(uint8_t*) (VIDEO_MEMORY + i * 2 + 1) = color;
#endif
    }

    cursorPos = 0;
    return;
}

void SwapBuffers() {
#if DOUBLE_BUFFERING
    for (uint64_t i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT * 2; i++)
        *(uint8_t*) (VIDEO_MEMORY + i) = videoBuffer[i];

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (cursorPos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((cursorPos >> 8) & 0xFF));
#endif
    return;
}

#define MAX_INPUT_LENGTH 256
#define MAX_INPUT_TEXT_COUNT 32

uint8_t inputText[MAX_INPUT_LENGTH + 1] = { '\0' };
uint8_t enteredTexts[MAX_INPUT_TEXT_COUNT][MAX_INPUT_LENGTH + 1];
uint16_t textCursorPos = 0, textHistoryPos = 0;
uint8_t lastScancode = 0;

bool leftShiftPressed = false;
bool rightShiftPressed = false;
bool capsLockState = false;

uint64_t strcmp(const char* str1, const char* str2) {
    for (uint64_t i = 0; true; i++)
        if (str1[i] != str2[i] || str1[i] == '\0' || str2[i] == '\0')
            return str1[i] - str2[i];
}

uint64_t strlen(const char* str) {
    uint64_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

void ExecuteCommand(const char* command) {
    if (strcmp(command, "halt") == 0) {
        asm volatile ("hlt");
    } else if (strcmp(command, "clear") == 0) {
        for (uint32_t i = 0; i < MAX_INPUT_TEXT_COUNT; i++) {
            for (uint32_t j = 0; j < MAX_INPUT_LENGTH + 1; j++) {
                if (enteredTexts[i][j] == '\0') break;
                enteredTexts[i][j] = '\0';
            }
        }
    } else {
        for (uint32_t i = 1; i < MAX_INPUT_TEXT_COUNT; i++) {
            for (uint32_t j = 0; j < MAX_INPUT_LENGTH + 1; j++) {
                enteredTexts[i - 1][j] = enteredTexts[i][j];
                enteredTexts[i][j] = '\0';
            }
        }

        uint32_t index = 0;
        const char* left = "Unknown command '";

        for (uint32_t i = 0; left[i] != '\0'; i++) {
            enteredTexts[MAX_INPUT_TEXT_COUNT - 1][index++] = left[i];
        }

        for (uint32_t i = 0; command[i] != '\0'; i++) {
            enteredTexts[MAX_INPUT_TEXT_COUNT - 1][index++] = command[i];
        }

        const char* right = "'.";

        for (uint32_t i = 0; i < MAX_INPUT_LENGTH + 1; i++) {
            enteredTexts[MAX_INPUT_TEXT_COUNT - 1][index++] = right[i];
            if (right[i] == '\0') break;
        }
    } return;
}

void KeyboardHandler(uint8_t scancode) {
    if (lastScancode == 0xE0) {
        lastScancode = scancode;

        switch (scancode) {
            case 0x50: {
                if (textHistoryPos - 1 >= 0 && (textHistoryPos - 1 == 0 || enteredTexts[MAX_INPUT_TEXT_COUNT - (textHistoryPos - 1)][0] != '\0')) {
                    textHistoryPos -= 1;
                } break;
            } case 0x48: {
                if (textHistoryPos + 1 < MAX_INPUT_TEXT_COUNT + 1 && enteredTexts[MAX_INPUT_TEXT_COUNT - (textHistoryPos + 1)][0] != '\0') {
                    textHistoryPos += 1;
                } break;
            } case 0x4B: {
                if (textCursorPos > 0) {
                    textCursorPos -= 1;
                } break;
            } case 0x4D: {
                if (textCursorPos < MAX_INPUT_LENGTH && inputText[textCursorPos] != '\0') {
                    textCursorPos += 1;
                } break;
            } case 0x53: {
                if (strlen((const char*) inputText) == textCursorPos)
                    break;

                for (uint16_t i = textCursorPos; i < MAX_INPUT_LENGTH; i++) {
                    inputText[i] = inputText[i + 1];
                    if (inputText[i] == '\0') break;
                } break;
            } default: {
                break;
            }
        }

        if (scancode == 0x50 || scancode == 0x48) {
            for (uint32_t i = 0; i < MAX_INPUT_LENGTH + 1; i++) {
                inputText[i] = textHistoryPos ? enteredTexts[MAX_INPUT_TEXT_COUNT - textHistoryPos][i] : '\0';
                textCursorPos = strlen((const char*) inputText);
            }
        } return;
    } else {
        lastScancode = scancode;
    }

    if (scancode < 0x3A && ScancodeLookupTable[scancode] != '\0') {
        uint64_t len = strlen((const char*) inputText);
        if (len >= MAX_INPUT_LENGTH) return;

        uint8_t prev = inputText[textCursorPos];
        inputText[textCursorPos] = ScancodeLookupTable[scancode];

        if ((leftShiftPressed || rightShiftPressed) != capsLockState) 
            if (inputText[textCursorPos] >= 'a' && inputText[textCursorPos] <= 'z')
                inputText[textCursorPos] -= 32;

        for (uint16_t i = ++textCursorPos; i < len + 2; i++) {
            uint8_t temp = inputText[i];
            inputText[i] = prev, prev = temp;
        } return;
    } else {
        switch (scancode) {
            case 0x0E: {
                if (strlen((const char*) inputText) == 0 || textCursorPos == 0)
                    break;

                for (uint16_t i = --textCursorPos; i < MAX_INPUT_LENGTH; i++) {
                    inputText[i] = inputText[i + 1];
                    if (inputText[i] == '\0') break;
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

                textCursorPos = 0, textHistoryPos = 0;
                ExecuteCommand((const char*) inputText);

                for (uint32_t i = 0; i < MAX_INPUT_LENGTH + 1; i++) {
                    inputText[i] = '\0';
                } break;
            } default: {
                break;
            }
        }
    }
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

static long seed = 1L;

void srand(unsigned int _seed) {
    seed = (long) _seed;
}

int rand() {
    seed = (seed * 214013L) + 2531011L;
    return (seed >> 16) & 0x7fff;
}

uint64_t RandInt(uint64_t min, uint64_t max) {
    return min + (rand() % (max - min + 1));
}

extern const char Text[];
extern uint64_t Text_Size;

extern "C" void Entry() {
    InitializeIDT();
    SetKeyboardHandler(KeyboardHandler);
    uint64_t x = 0, y = 0, i = 0;

    while (true) {
        Clear();
        WriteString(Text);
        WriteString("\nTicks: ");
        WriteString(IntToString(x++, nullptr, 10));
        WriteString("\nRandom Number: ");
        WriteString(IntToString(y, nullptr, 10));
        WriteString("\n");
        seed++;

        if (i++ > 1024)
            y = RandInt(10, 100), i = 0;

        for (uint64_t i = MAX_INPUT_TEXT_COUNT - 20; i < MAX_INPUT_TEXT_COUNT; i++) {
            if (enteredTexts[i][0] == '\0') continue;
            WriteString((const char*) enteredTexts[i]);
            WriteString("\n");
        }

        WriteString("> ");
        uint16_t pos = GetCursorPosition();
        WriteString((const char*) inputText);
        WriteString("\nCursor: ");
        WriteString(IntToString(textCursorPos, nullptr, 10));
        SetCursorPosition(pos + textCursorPos);
        SwapBuffers();
    }
}
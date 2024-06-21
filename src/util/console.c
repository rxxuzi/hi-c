// colors.h
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "console.h"

static void set(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

static void del() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void c_R(const char *format, ...) {
    set(FOREGROUND_RED | FOREGROUND_INTENSITY);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    del();
}

void c_G(const char *format, ...) {
    set(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    del();
}

void c_B(const char *format, ...) {
    set(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    del();
}

void c_C(const char *format, ...) {
    set(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    del();
}

void c_Y(const char *format, ...) {
    set(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    del();
}

void c_M(const char *format, ...) {
    set(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    del();
}


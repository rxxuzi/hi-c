// console.h
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

void c_R(const char *format, ...);
void c_Y(const char *format, ...);
void c_G(const char *format, ...);
void c_B(const char *format, ...);
void c_M(const char *format, ...);
void c_C(const char *format, ...);

#ifdef __cplusplus
}
#endif
#endif // CONSOLE_H

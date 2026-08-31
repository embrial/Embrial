#ifndef ASSERTIONS_H
#define ASSERTIONS_H
/*
Header comment:
    File:       assertions.h
    Project:    lang
    Created:    2026-08-21

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/

#ifdef LANG_RELEASE
#define assert()
#define assertEq()
#elif
#define assert(expr) do {                                   \
    if (!(expr)) {                                          \
        fatal("Assertion '" #expr "' failed.");             \
    }                                                       \
} while (0)
#define assertEq(x, y, xf, yf) do {                         \
    __typeof__(x) _x = (x);                                 \
    __typeof__(y) _y = (y);                                 \
    if (_x != _y) {                                         \
        fatal("Assertion '" #x " == " #y "' failed.\n"      \
              "   expected: " xf "\n"                       \
              "   actual:   " yf,                           \
              _x, _y);                                      \
    }                                                       \
} while (0)
#endif

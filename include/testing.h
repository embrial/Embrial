#ifndef TESTING_H
#define TESTING_H
/*
Header comment:
    File:       testing.h
    Project:    TBD
    Created:    2026-08-21

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/


#ifdef LANG_TESTING_IMPLEMENTATION
    size_t assertionsTotal = 0;
    size_t assertionsFailed = 0;
    bool previousTestEnded = true;
#else
    extern size_t assertionsTotal;
    extern size_t assertionsFailed;
    extern bool previousTestEnded;
#endif

#define assert(expr) do {                               \
    assertionsTotal += 1;                                   \
    info("assert '" #expr "'");                             \
    if (!(expr)) {                                          \
        assertionsFailed += 1;                              \
        fatal("Assertion failed.");                         \
    } else info ("Assertion passed.");                      \
} while (0)
#define assertEq(x, y, xf, yf) do {                     \
    assertionsTotal += 1;                                   \
    __typeof__(x) _x = (x);                                 \
    __typeof__(y) _y = (y);                                 \
    info(   "assert '" #x " == " #y "'\n"                   \
            "   expecting: " xf "\n"                        \
            "   actual:    " yf,                            \
          _x, _y);                                          \
    if (_x != _y) {                                         \
        assertionsFailed += 1;                              \
        fatal("Assertion failed.");                         \
    } else info("Assertion passed.");                       \
} while (0)
#define beginTest(name) do {                                \
    if (!previousTestEnded) assertionsStatus();             \
    info("---------- %s ----------", name);                 \
    assertionsTotal = 0;                                    \
    assertionsFailed = 0;                                   \
    previousTestEnded = false;                              \
} while (0)
#define assertionsStatus() do {                             \
    info("Assertions: %zu, Passed: %zu Failed: %zu",        \
        assertionsTotal,                                    \
        assertionsTotal - assertionsFailed,                 \
        assertionsFailed);                                  \
    previousTestEnded = true;                               \
} while (0)
#endif

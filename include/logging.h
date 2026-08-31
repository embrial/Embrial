#ifndef LOGGING_H
#define LOGGING_H

/*
Header comment:
    File:       logging.h
    Project:    lang
    Created:    2026-08-21

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/

#define descriptor(label, ...) do {                         \
    fprintf(stderr, "%s:%5d %s: ",                          \
            __FILE__, __LINE__, label);                     \
    fprintf(stderr, __VA_ARGS__);                           \
    fputc('\n', stderr);                                    \
} while (0)

#define fatal(...) do {                                     \
    descriptor("FATAL", __VA_ARGS__);                       \
    exit(EXIT_FAILURE);                                     \
} while (0)

#ifndef LANG_RELEASE
#define info(...) do {                                      \
    descriptor("INFO", __VA_ARGS__);                        \
} while (0)                                       
#endif

#endif

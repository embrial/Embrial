/*
Header comment:
    File:       langType.h
    Project:    TBD
    Created:    2026-08-27

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/


typedef unsigned long long U64;
typedef unsigned char U8;

U64 randomU64(void) {
    seed += 0x9E3779B97F4A7C15ULL;

    U64 z = seed;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

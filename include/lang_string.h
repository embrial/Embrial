/*
Header comment:
    File:       lang_string.h
    Project:    TBD
    Created:    2026-08-19

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:
    String is a lightweight, non-owning string type consisting of a pointer to
    character data and an explicit length. It provides a length-aware
    alternative to working directly with null-terminated C strings.

    String does not own the memory referenced by its data pointer and does not
    allocate or free that memory. The caller is responsible for ensuring that
    the underlying data remains valid for the lifetime of the String.

    The implementation provides functions for constructing a String from a C
    string, accessing characters by index, calculating a platform-sized FNV-1a
    hash, and comparing Strings for equality.

Usage:
    String str1 = String_fromCString("hello");
    
    size_t hash = String_hash(str1);

    String str2 = String_fromCString("hello");

    if (String_equals(str1, str2)) {
        printf("str1 == str2");
    }
*/

// Includes:
#include <string.h>

// Structs:
typedef struct {
    const char *data;
    size_t length;
} String;

// Functions:
static inline String String_fromCString(const char *data) {
    return (String){data, strlen(data)};
}

static inline size_t String_hash(String string) {
    #if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    // 64-bit FNV-1a constants
    size_t hash = 14695981039346656037ULL;
    const size_t prime = 1099511628211ULL;
    #else
    // 32-bit FNV-1a constants
    size_t hash = 2166136261U;
    const size_t prime = 16777619U;
    #endif

    for (size_t i = 0; i < string.length; i++) {
        hash ^= (unsigned char)string.data[i];
        hash *= prime;
    }

    return hash;
}

static inline bool String_equals(String str1, String str2) {
    if (str1.length != str2.length) return false;
    return memcmp(str1.data, str2.data, str1.length) == 0;
}

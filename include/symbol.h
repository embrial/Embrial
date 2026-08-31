#ifndef SYMBOL_H
#define SYMBOL_H
/*
Header comment:
    File:       symbol.h
    Project:    lang
    Created:    2026-08-21

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/

// Constants:
#define MIN_SYMBOL_TABLE_ENTRIES 1024

// Data structures:
typedef size_t SymbolId;

typedef enum {
    SYMBOL_IDENTIFER,
    SYMBOL_LAMBDA,
    SYMBOL_RESOLVED,
    SYMBOL_UNRESOLVED
} SymbolKind;

typedef struct Symbol Symbol;

struct Symbol {
    SymbolKind kind;

    // metadata
    String name;
    size_t hash;
    SymbolId id;

    bool constant;                  // identifier / lambda
    union {
        Symbol *type;               // identifier
        AstNode *node;              // lambda / resolved
        Constraint *constraints;    // unresolved
    };
};

// typedef Arena SymbolArena;
// typedef Arena SymbolTableArena;
typedef struct {
    Symbol **entries;
    size_t count;
    size_t capacity;
    bool ok;
} SymbolTable;

void SymbolTable_init(SymbolTable *table, size_t capacity);
void SymbolTable_free(SymbolTable *table);
bool SymbolTable_grow(SymbolTable *table);
bool SymbolTable_insert(SymbolTable *table, Symbol *symbol);
Symbol *SymbolTable_get(const SymbolTable *table, String name);
#endif

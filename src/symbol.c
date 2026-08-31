#include "symbol.h"

// Function definitions:
void SymbolTable_init(SymbolTable *table, size_t capacity) {
    table->ok = false;
    if (capacity < MIN_SYMBOL_TABLE_ENTRIES)
        capacity = MIN_SYMBOL_TABLE_ENTRIES;

    table->entries = calloc(capacity, sizeof(Symbol *));
    if (!table->entries) return;
    
    table->count = 0;
    table->capacity = capacity;

    table->ok = true;
}
void SymbolTable_free(SymbolTable *table) {
    free(table->entries);

    table->entries = NULL;
    table->count = 0;
    table->capacity = 0;
}
bool SymbolTable_grow(SymbolTable *table) {
    if (!table->ok) return false;
    size_t capacity = table->capacity;
    if (capacity < MIN_SYMBOL_TABLE_ENTRIES)
        capacity = MIN_SYMBOL_TABLE_ENTRIES;
    else capacity *= 2;

    Symbol **newEntries = calloc(capacity, sizeof(Symbol *));
    if (!newEntries) {
        table->ok = false;
        return false;
    }

    Symbol *oldEntry;
    for (size_t i = 0; i < table->capacity; i++) {
        oldEntry = table->entries[i];
        if (!oldEntry) continue;

        size_t index = entry->hash % capacity;
        size_t j = 0;
        for (; j < capacity; j++) {
            if (!newEntries[index]) break;
            index = (index + 1) % capacity;
        }

        assert(j != capacity);

        newEntries[index] = oldEntry;
    }

    free(table->entries);

    table->entries = newEntries;
    table->capacity = newCapacity;
}
bool SymbolTable_insert(SymbolTable *table, Symbol *symbol) {
    if (!table->ok) return false;
    if ((table->count + 1) * 10 >= table->capacity * 7) {
        if (!SymbolTable_grow(table)) return false;
    }

    size_t index = symbol->hash % table->capacity;

    Symbol *entry;
    size_t i = 0;
    for (; i < table->capacity; i++) {
        entry = table->entries[index];
        if (!entry) break;

        if (entry->hash == symbol->hash &&
            String_equals(entry->name, symbol->name)
        ) {
            return false;
        }

        index = (index + 1) % table->capacity;
    }

    assert(i != table->capacity);

    tables->entries[index] = symbol;

    table->count++;
    return true;
}
Symbol *SymbolTable_get(const SymbolTable *table, String name) {
    if (!table->ok) return NULL;
    size_t hash = String_hash(name);
    size_t index = hash % table->capacity;

    Symbol *entry;
    // not using while true because then that presents an infinte loop
    size_t i = 0;
    for (; i < table->capacity; i++) {
        entry = table->entries[index];
        if (!entry) break;

        if (entry->hash == hash && String_equals(entry->name, name)) {
            return entry;
        }

        index = (index + 1) % table->capacity;
    }

    return NULL;
}



#endif

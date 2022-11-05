#pragma once
#include "foundation/basic.h"

typedef struct Atom_Table Atom_Table;

typedef struct Atom {
    uint64_t hash;
    String8 str;
} Atom;

Atom_Table *atom_table_create(uint64_t capacity);
void atom_table_destroy(Atom_Table *table);

Atom *atom_add(Atom_Table *table, const char *str, uint32_t len);
Atom *atom_find(Atom_Table *table, const char *str);

static inline bool atoms_match(Atom *lhs, Atom *rhs)
{
    return lhs->hash == rhs->hash;
}

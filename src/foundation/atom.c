#include "atom.h"
#include "foundation/allocator.h"
#include "foundation/hash.h"
#include "foundation/murmur_hash64.h"

struct Atom_Table {
    Allocator allocator;
    Hash lookup;
};

Atom_Table *atom_table_create(uint64_t capacity)
{
    Allocator allocator = allocator_create_fixed_vm(capacity);
    Atom_Table *table = c_alloc(&allocator, sizeof(*table));
    table->allocator = allocator;
    table->lookup = (Hash) { 0 };
    return table;
}

void atom_table_destroy(Atom_Table *table)
{
    Allocator *a = &table->allocator;
    hash_free(&table->lookup, a);
    c_free(a, table, (uint64_t)a->user_data);
}

Atom *atom_add(Atom_Table *table, const char *str, uint32_t len)
{
    uint64_t key = murmur_hash64a(str, len, 0);

    Atom *res = (Atom *)hash_get(&table->lookup, key);
    if (res != 0)
        return res;

    // Header + string len + terminator
    uint64_t needed_size = sizeof(Atom) + len + 1;
    Atom *atom = c_alloc(&table->allocator, needed_size);
    memset(atom, 0, needed_size);
    atom->hash = key;
    atom->str.len = len;
    atom->str.data = (uint8_t *)atom + sizeof(Atom);
    memcpy((uint8_t *)atom->str.data, str, len);

    // Store pointer in lookup table
    hash_add(&table->lookup, key, (uint64_t)atom, &table->allocator);
    return atom;
}

Atom *atom_find(Atom_Table *table, const char *str)
{
    uint64_t key = murmur_hash64a_string(str);
    return (Atom *)hash_get(&table->lookup, key);
}

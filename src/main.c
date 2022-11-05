#pragma once
#include "foundation/basic.h"
#include "foundation/array.h"
#include "foundation/atom.h"
#include "foundation/os_helper.h"
#include "lexer.h"
#include "token_util.h"

int main(int argc, char **argv) {
    Atom_Table *atoms = atom_table_create(MB(16));
    Token *tokens = 0;

    uint64_t start_time = os_time_now();
    const char *path = argc > 1 ? argv[1] : "first.ps";
    lexer_read_file(path, &tokens, atoms, system_allocator);
    double delta = os_time_delta(os_time_now(), start_time);

    print_token_stream(&tokens);
    
    printf("Parsed %zu tokens in %.4fs.\n", array_size(tokens), delta);
    printf("Token stream size = %.2fKB (1 token is %zu bytes)", 
        (sizeof(Token) * array_size(tokens)) / 1000.f, sizeof(Token));

    atom_table_destroy(atoms);
    array_free(tokens, system_allocator);
    
    return 0;
}

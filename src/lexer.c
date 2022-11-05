#include "lexer.h"
#include "foundation/atom.h"
#include "foundation/array.h"
#include "foundation/allocator.h"
#include "foundation/atom.h"
#include "foundation/os_helper.h"
#include <ctype.h>

typedef struct Lexer {
    uint8_t *data;
    uint64_t size;
    uint64_t cursor;
    int current_line;
    int current_char;
    Token **tokens;
    Atom_Table *atoms;
    Allocator *allocator;
    uint8_t scratch_buf[512];
} Lexer;

static inline Token *make_token(Token_Type type, Lexer *lexer)
{
    Token token = { 
        .type = type, 
        .l0 = lexer->current_line, 
        .c0 = lexer->current_char 
    };
    array_push(*lexer->tokens, token, lexer->allocator);
    return &(*lexer->tokens)[array_size(*lexer->tokens) - 1];
}

static char peek_next_char(Lexer *l)
{
    if (l->cursor >= l->size)
        return -1;
    return l->data[l->cursor];
}

static char peek_char(Lexer *l, int lookahead)
{
    int64_t idx = (int64_t)l->cursor + lookahead;
    if (idx < 0 || idx >= (int64_t)l->size)
        return -1;
    return l->data[idx];
}

static void eat_char(Lexer *l)
{
    if (l->data[l->cursor] == '\n') {
        l->current_line++;
        l->current_char = 0;
    }
    l->cursor++;
    l->current_char++;
}

static void read_identifier(Lexer *l)
{
    Token *token = make_token(TOKEN_IDENTIFIER, l);

    uint32_t num_chars = 0;
    char c = peek_next_char(l);
    while (isalnum(c) || c == '_') {
        l->scratch_buf[num_chars++] = c;
        eat_char(l);
        c = peek_next_char(l);
    }
    l->scratch_buf[num_chars] = 0;

    // Lazy check for keywords
    const uint32_t num_keywords = TOKEN_KEYWORD_LAST - TOKEN_KEYWORD_IF;
    for (uint32_t i = 0; i < num_keywords; ++i) {
        if (strcmp(l->scratch_buf, keyword_strings[i]) == 0) {
            token->type = TOKEN_KEYWORD_IF + i;
            break;
        }
    }

    if (token->type == TOKEN_IDENTIFIER) {
        Atom *atom = atom_add(l->atoms, l->scratch_buf, num_chars);
        token->name = atom;
    }

    token->l1 = l->current_line;
    token->c1 = l->current_char;
}

static void read_number(Lexer *l)
{
    Token *token = make_token(TOKEN_NUMBER, l);
    uint32_t num_chars = 0;
    char c = peek_next_char(l);

    if (c == '0' && peek_char(l, 1) == 'x') {
        // Parse hexadecimal
        eat_char(l);
        eat_char(l);
        c = peek_next_char(l);
        while (isxdigit(c)) {
            l->scratch_buf[num_chars++] = c;
            eat_char(l);
            c = peek_next_char(l);
        }
        l->scratch_buf[num_chars] = 0;
        token->int_value = strtoull(l->scratch_buf, NULL, 16);
    }
    else if (c == '0' && peek_char(l, 1) == 'b') {
        // Parse binary
        eat_char(l);
        eat_char(l);
        c = peek_next_char(l);
        while (c == '0' || c == '1') {
            l->scratch_buf[num_chars++] = c;
            eat_char(l);
            c = peek_next_char(l);
        }
        l->scratch_buf[num_chars] = 0;
        token->int_value = strtoull(l->scratch_buf, NULL, 2);
    }
    else {
        bool is_float = false;
        while (isdigit(c) || c == '.') {
            is_float |= (c == '.');
            l->scratch_buf[num_chars++] = c;
            eat_char(l);
            c = peek_next_char(l);
        }
        l->scratch_buf[num_chars] = 0;
        if (is_float)
            token->float_value = strtod(l->scratch_buf, NULL);
        else
            token->int_value = strtoull(l->scratch_buf, NULL, 10);
    }

    token->l1 = l->current_line;
    token->c1 = l->current_char;
}

static void read_comment(Lexer *l)
{
    eat_char(l);
    char c = peek_next_char(l);
    bool single_line = (c == '/');
    bool multi_line  = (c == '*');
    eat_char(l);

    while ((c = peek_next_char(l)) >= 0) {
        eat_char(l);
        if (single_line && c == '\n')
            break;
        if (multi_line && c == '*' && peek_next_char(l) == '/') {
            eat_char(l);
            break;
        }
    }
}

static void read_string(Lexer *l)
{
    char end_symbol = peek_next_char(l);
    eat_char(l);

    Token *token = make_token(TOKEN_STRING, l);

    char c;
    uint32_t num_chars = 0;
    while ((c = peek_next_char(l)) >= 0) {
        eat_char(l);
        if (c == end_symbol) {
            // End parsing string when the end symbol is seen
            // Ignore if it's an escape sequence (\' or \")
            bool escape_sequence = peek_char(l, -2) == '\\' && peek_char(l, -3) != '\\';
            if (!escape_sequence)
                break;
        }
        l->scratch_buf[num_chars++] = c;
    }

    Atom *atom = atom_add(l->atoms, l->scratch_buf, num_chars);
    token->string_value = atom->str;

    token->l1 = l->current_line;
    token->c1 = l->current_char;
}

static void read_symbol(Lexer *l)
{
    char lhs = peek_next_char(l);

    Token_Type type = TOKEN_ERROR;
    Token *token = make_token(type, l);
    eat_char(l);

    char rhs = peek_next_char(l);

    switch (lhs) {
        case '+':
            if (rhs == '=') type = TOKEN_PLUS_EQUALS;
            break;
        case '-':
            if (rhs == '>') type = TOKEN_RIGHT_ARROW;
            if (rhs == '=') type = TOKEN_MINUS_EQUALS;
            break;
        case '*':
            if (rhs == '=') type = TOKEN_MUL_EQUALS;
            break;
        case '/':
            if (rhs == '=') type = TOKEN_DIV_EQUALS;
            break;
        case '%':
            if (rhs == '=') type = TOKEN_MOD_EQUALS;
            break;
        case '=':
            if (rhs == '=') type = TOKEN_IS_EQUAL;
            break;
        case '!':
            if (rhs == '=') type = TOKEN_IS_NOT_EQUAL;
            break;
        case '&':
            if (rhs == '&') type = TOKEN_LOGICAL_AND;
            if (rhs == '=') type = TOKEN_BITWISE_AND_EQUALS;
            break;
        case '|':
            if (rhs == '|') type = TOKEN_LOGICAL_OR;
            if (rhs == '=') type = TOKEN_BITWISE_OR_EQUALS;
            break;
        case '^':
            if (rhs == '=') type = TOKEN_BITWISE_XOR_EQUALS;
            break;
        case '<':
            if (rhs == '=') type = TOKEN_LESS_EQUALS;
            if (rhs == '<') type = TOKEN_SHIFT_LEFT;
            break;
        case '>':
            if (rhs == '=') type = TOKEN_GREATER_EQUALS;
            if (rhs == '>') type = TOKEN_SHIFT_RIGHT;
            break;
    }

    if (type != TOKEN_ERROR) {
        token->type = type;
        // Consume extra character that was part of this symbol
        eat_char(l);
    } else {
        // Implicit conversion to Token_Type (0-255)
        token->type = lhs;
    }

    token->l1 = l->current_line;
    token->c1 = l->current_char;
}

void lexer_read_file(const char *path, Token **token_stream, Atom_Table *atoms, Allocator *allocator)
{
    uint64_t size = 0;
    uint8_t *data = os_read_entire_file(path, &size, allocator);

    if (data == 0) {
        printf("Unable to read file: '%s'\n", path);
        return;
    }

    Lexer *lexer = &(Lexer) {
        .data = data,
        .size = size,
        .tokens = token_stream,
        .atoms = atoms,
        .allocator = allocator,
    };

    array_reset(*token_stream);
    array_ensure(*lexer->tokens, 256, lexer->allocator);

    char c;
    while ((c = peek_next_char(lexer)) >= 0) {
        if (isalpha(c) || c == '_') {
            // Read a text which is not a string literal
            // E.g. keywords, variables, function names, parameters..
            read_identifier(lexer);
        } else if (isdigit(c)) {
            // Read integer or floating point number
            read_number(lexer);
        } else if (c == '/') {
            // Discard single or multi-line comments
            read_comment(lexer);
        } else if (c == '\'' || c == '\"') {
            // Read string literal encapsulated within '' or ""
            read_string(lexer);
        } else if (isgraph(c)) {
            // Read remaining symbols
            // Also checks for nearby characters and consumes them accordingly
            // E.g. +=, >=, ->, &&..
            read_symbol(lexer);
        } else {
            // Skip characters that are not of interest
            eat_char(lexer);
        }
    }

    c_free(allocator, data, size);
}

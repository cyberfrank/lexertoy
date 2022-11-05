#pragma once
#include "foundation/basic.h"

struct Atom;
struct Atom_Table;
struct Allocator;

typedef enum Token_Type {
    // ASCII characters 0-255

    TOKEN_IDENTIFIER = 256,
    TOKEN_NUMBER = 257,
    TOKEN_STRING = 258,

    TOKEN_PLUS_EQUALS = 259,
    TOKEN_MINUS_EQUALS = 260,
    TOKEN_MUL_EQUALS = 261,
    TOKEN_DIV_EQUALS = 262,
    TOKEN_MOD_EQUALS = 263,
    TOKEN_IS_EQUAL = 264,
    TOKEN_IS_NOT_EQUAL = 265,
    TOKEN_LOGICAL_AND = 266,
    TOKEN_LOGICAL_OR = 267,
    TOKEN_LESS_EQUALS = 268,
    TOKEN_GREATER_EQUALS = 269,

    TOKEN_RIGHT_ARROW = 270, // -> 
    
    TOKEN_SHIFT_LEFT = 271,
    TOKEN_SHIFT_RIGHT = 272,
    TOKEN_BITWISE_AND_EQUALS = 273,
    TOKEN_BITWISE_OR_EQUALS = 274,
    TOKEN_BITWISE_XOR_EQUALS = 275,

    // Keywords
    TOKEN_KEYWORD_IF = 276,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_THEN,
    TOKEN_KEYWORD_CASE,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_ENUM,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_BREAK,
    TOKEN_KEYWORD_CONTINUE,
    TOKEN_KEYWORD_SWITCH,
    TOKEN_KEYWORD_DEFER,
    TOKEN_KEYWORD_SIZE_OF,
    TOKEN_KEYWORD_TYPE_OF,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,
    TOKEN_KEYWORD_NULL,
    TOKEN_KEYWORD_LAST,

    TOKEN_ERROR,
} Token_Type;

static const char *keyword_strings[] = {
    "if", "else", "then", "case", "return", "struct",
    "enum", "while", "break", "continue", "defer", "switch",
    "sizeof", "typeof", "true", "false", "null",
};

typedef struct Token {
    Token_Type type;
    int l0, c0, l1, c1;
    union {
        struct Atom *name;
        uint64_t int_value;
        double float_value;
        String8 string_value;
    };
} Token;

// 
// Read file at `path` and parse its data into a stream of tokens `token_stream`
// Any parsed identifiers and strings are added to the Atom_Table
//
void lexer_read_file(const char *path, Token **token_stream, struct Atom_Table *atoms, struct Allocator *allocator);


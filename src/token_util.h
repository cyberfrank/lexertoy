#pragma once
#include "foundation/basic.h"
#include "foundation/atom.h"
#include "lexer.h"
#include <math.h>

static const char *token_type_name(Token token)
{
    if (token.type < 256) {
        return (const char *)&token.type;
    }
    else if (token.type >= TOKEN_KEYWORD_IF && token.type < TOKEN_KEYWORD_LAST) {
        return keyword_strings[token.type - TOKEN_KEYWORD_IF];
    } else {
        switch (token.type) {
        case TOKEN_IDENTIFIER:      return "Ident";
        case TOKEN_NUMBER:          return "Number";
        case TOKEN_STRING:          return "String";
        case TOKEN_PLUS_EQUALS:     return "+=";
        case TOKEN_MINUS_EQUALS:    return "-=";
        case TOKEN_MUL_EQUALS:      return "*=";
        case TOKEN_DIV_EQUALS:      return "/=";
        case TOKEN_MOD_EQUALS:      return "\\%=";
        case TOKEN_IS_EQUAL:        return "==";
        case TOKEN_IS_NOT_EQUAL:    return "!=";
        case TOKEN_LOGICAL_AND:     return "&&";
        case TOKEN_LOGICAL_OR:      return "||";
        case TOKEN_LESS_EQUALS:     return "<=";
        case TOKEN_GREATER_EQUALS:  return ">=";
        case TOKEN_RIGHT_ARROW:     return "->";
        case TOKEN_SHIFT_LEFT:      return ">>";
        case TOKEN_SHIFT_RIGHT:     return "<<";
        case TOKEN_BITWISE_AND_EQUALS: return "&=";
        case TOKEN_BITWISE_OR_EQUALS:  return "|=";
        case TOKEN_BITWISE_XOR_EQUALS: return "^=";
        default:                       return "(Invalid)";
        }
    }
}


static void print_token_stream(Token **tokens)
{
    int max_line = 0;
    int max_char = 0;
    for (Token *it = *tokens; it != array_end(*tokens); ++it) {
        if (it->l0 > max_line) max_line = it->l0 + 1;
        if (it->l1 > max_line) max_line = it->l1 + 1;
        if (it->c0 > max_char) max_char = it->c0;
        if (it->c1 > max_char) max_char = it->c1;
    }
    
    int pad = 6;
    pad += ((int)log10l(max_line) + 1) * 2;
    pad += ((int)log10l(max_char) + 1) * 2;

    for (Token *it = *tokens; it != array_end(*tokens); ++it) {
        char buf[256];
        int len = snprintf(buf, 256, "[%i,%i->%i,%i]", it->l0 + 1, it->c0, it->l1 + 1, it->c1);
        const char *dashes = "--------------------------------";
        printf ("%s %.*s-> ", buf, pad - len, dashes);

        if (it->type == TOKEN_NUMBER) {
            printf("%s <%zu|%f>\n", token_type_name(*it), it->int_value, it->float_value);
        }
        else if (it->type == TOKEN_STRING) {
            printf("%s \"%s\"\n", token_type_name(*it), it->string_value.data);
        }
        else if (it->type == TOKEN_IDENTIFIER) {
            printf("%s '%s'\n", token_type_name(*it), it->name->str.data);
        }
        else {
            printf("%s\n", token_type_name(*it));
        }
    }
}

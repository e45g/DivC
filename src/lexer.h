#ifndef _LEXER_H
#define _LEXER_H

typedef enum token_type {
    // literals
    IDENTIFIER,
    NUMBER,
    STRING,

    // arithmetic operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    MOD,

    PLUS_EQ,
    MINUS_EQ,
    STAR_EQ,
    SLASH_EQ,
    MINUS_MINUS,
    PLUS_PLUS,
    ASSIGN,

    // logic operators,
    EQUAL,
    GREATER,
    GREATER_EQ,
    LESS,
    LESS_EQ,
    NOT_EQ,
    
    // bit operators
    NOT,
    OR,
    OR_EQ,
    AND,
    AND_EQ,
    XOR,
    XOR_EQ,

    // keywords
    VOID,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
    UNSIGNED,
    LONG,
    SHORT,
    RETURN,

    // special
    LEFT_PAREN,
    LEFT_CURLY,
    LEFT_SQUARE,
    RIGHT_PAREN,
    RIGHT_CURLY,
    RIGHT_SQUARE,
    SEMICOLON,
    ARROW,
    COMMA,
    DOT,

    TOKEN_EOF,
    TOKEN_UNKNOWN
} token_type_t;

typedef struct token {
    token_type_t type;
    char *value;
    struct token *next;
} token_t;

typedef struct keyword_entry {
    char *word;
    token_type_t type;
} keyword_entry_t;

token_t *lexer_parse(char *src);

#endif

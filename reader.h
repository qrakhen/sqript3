#ifndef sqript_reader_h
#define sqript_reader_h

typedef enum {
    TOKEN_GROUP_OPEN, 
    TOKEN_GROUP_CLOSE,

    TOKEN_CONTEXT_OPEN, 
    TOKEN_CONTEXT_CLOSE,

    TOKEN_ARRAY_OPEN, 
    TOKEN_ARRAY_CLOSE,
    TOKEN_ARRAY_ADD,
    TOKEN_ARRAY_REMOVE,

    TOKEN_COMMA, 
    TOKEN_DOT, 
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    
    TOKEN_MINUS, 
    TOKEN_PLUS,
    TOKEN_DECREMENT,
    TOKEN_INCREMENT,

    TOKEN_SLASH, 
    TOKEN_STAR,

    TOKEN_BITWISE_AND, 
    TOKEN_BITWISE_OR, 
    TOKEN_BITWISE_XOR, 
    TOKEN_BITWISE_NOT,

    TOKEN_BITWISE_LEFT, 
    TOKEN_BITWISE_RIGHT,

    TOKEN_BANG, 
    TOKEN_BANG_EQUAL,

    TOKEN_ASSIGN, 
    TOKEN_EQUAL,

    TOKEN_GREATER, 
    TOKEN_GREATER_EQUAL,

    TOKEN_LESS, 
    TOKEN_LESS_EQUAL,

    TOKEN_IDENTIFIER, 
    TOKEN_STRING, 
    TOKEN_NUMBER,

    TOKEN_AND, 
    TOKEN_CLASS, 
    TOKEN_ELSE, 
    TOKEN_FALSE,
    TOKEN_FOR, 
    TOKEN_FUNCTION, 
    TOKEN_IF, 
    TOKEN_NULL, 
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_TYPEOF,
    TOKEN_RETURN, 
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE, 
    TOKEN_VAR, 
    TOKEN_WHILE,
    TOKEN_ASSIGN_REF,

    TOKEN_ERROR, 
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

void initScanner(const char* source);
Token readToken();

#endif

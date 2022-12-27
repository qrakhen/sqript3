#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "digester.h"
#include "array.h"
#include "memory.h"
#include "reader.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool failed;
    bool panic;
} Digester;

typedef enum {
    W_NONE,
    W_ASSIGN,
    W_OR,
    W_AND,
    W_EQUALS,
    W_COMPARE,
    W_TERM,
    W_FACTOR,
    W_UNARY,
    W_CALL,
    W_PRIMARY
} Weight;

typedef void (*WeightCallback)(bool canAssign);

typedef struct {
    WeightCallback prefix;
    WeightCallback infix;
    Weight weight;
} WeightRule;

typedef struct {
    Token name;
    int depth;
    bool isCaptured;
} Local;

typedef struct {
    byte index;
    bool isLocal;
} Upvalue;

typedef enum {
    F_FUNC,
    F_INIT,
    F_INST,
    F_CODE
} FunctionType;

typedef struct Compiler {
    struct Compiler* enclosing;
    PtrFunq* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    Upvalue upvalues[UINT8_COUNT];
    int scopeDepth;
} Compiler;

typedef struct ClassCompiler {
    struct ClassCompiler* enclosing;
    bool inherited;
} ClassCompiler;

Digester digester;
Compiler* current = NULL;
ClassCompiler* currentClass = NULL;

static Segment* currentSegment() {
    return &current->function->segment;
}

static void errorAt(Token* token, const char* message) {
    if (digester.panic) return;
    digester.panic = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    digester.failed = true;
}

static void error(const char* message) {
    errorAt(&digester.previous, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&digester.current, message);
}

static void next() {
    digester.previous = digester.current;

    for (;;) {
        digester.current = readToken();
        if (digester.current.type != TOKEN_ERROR) break;

        errorAtCurrent(digester.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (digester.current.type == type) {
        next();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return digester.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    next();
    return true;
}

static void emitByte(byte byte) {
    writeSegment(currentSegment(), byte, digester.previous.line);
}

static void emitBytes(byte byte1, byte byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentSegment()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("loop size (way) over 9000.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static int emitJump(byte instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentSegment()->count - 2;
}

static void emitReturn() {

    if (current->type == F_INIT) {
        emitBytes(OP_GET_LOCAL, 0);
    } else {
        emitByte(OP_NULL);
    }

    emitByte(OP_RETURN);
}

static byte makeConstant(Value value) {
    int constant = registerConstant(currentSegment(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (byte)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
    int jump = currentSegment()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    currentSegment()->code[offset] = (jump >> 8) & 0xff;
    currentSegment()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;
    if (type != F_CODE) {
        current->function->name = copyString(digester.previous.start, digester.previous.length);
    }

    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;

    if (type != F_FUNC) {
        local->name.start = "this";
        local->name.length = 4;
    } else {
        local->name.start = "";
        local->name.length = 0;
    }
}

static PtrFunq* endCompiler() {
    emitReturn();
    PtrFunq* function = current->function;

    #ifdef DEBUG_PRINT_CODE
    if (!digester.failed) {
        disassembleChunk(currentSegment(), function->name != NULL
                         ? function->name->chars : "<script>");
    }
    #endif

    current = current->enclosing;
    return function;
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;

    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth >
           current->scopeDepth) {

        if (current->locals[current->localCount - 1].isCaptured) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        }
        current->localCount--;
    }
}

static void expression();
static void statement();
static void declaration();
static WeightRule* getRule(TokenType type);
static void digestWeight(Weight precedence);

static byte identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name->start,
                        name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static int addUpvalue(Compiler* compiler, byte index,
                      bool isLocal) {
    int upvalueCount = compiler->function->upvalueCount;

    for (int i = 0; i < upvalueCount; i++) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalueCount == UINT8_COUNT) {
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolveLocal(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (byte)local, true);
    }

    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(compiler, (byte)upvalue, false);
    }

    return -1;
}

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;

    local->depth = -1;
    local->isCaptured = false;
}

static void declareVariable() {
    if (current->scopeDepth == 0) return;

    Token* name = &digester.previous;
    for (int i = current->localCount - 1; i >= 0; i--) {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

static byte parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0) return 0;

    return identifierConstant(&digester.previous);
}

static void markInitialized() {
    if (current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth =
        current->scopeDepth;
}

static void defineVariable(byte global) {
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

static byte argumentList() {
    byte argCount = 0;
    if (!check(TOKEN_GROUP_CLOSE)) {
        do {
            expression();
            if (argCount == 255) {
                error("which function on this planet would require more than 255 arguments?");
            }
            argCount++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_GROUP_CLOSE, "expected ) after argument list");
    return argCount;
}

static void __AND(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    digestWeight(W_AND);

    patchJump(endJump);
}

static void __BIN(bool canAssign) {
    TokenType operatorType = digester.previous.type;
    WeightRule* rule = getRule(operatorType);
    digestWeight((Weight)(rule->weight + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        case TOKEN_BITWISE_AND:   emitByte(OP_BITWISE_AND); break;
        case TOKEN_BITWISE_OR :   emitByte(OP_BITWISE_OR); break;
        case TOKEN_BITWISE_XOR:   emitByte(OP_BITWISE_XOR); break;
        case TOKEN_BITWISE_NOT:   emitByte(OP_BITWISE_NOT); break;
        default: return;
    }
}

static void __CAL(bool canAssign) {
    byte argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

static void __DOT(bool canAssign) {
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    byte name = identifierConstant(&digester.previous);

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_PROPERTY, name);
    } else if (match(TOKEN_GROUP_OPEN)) {
        byte argCount = argumentList();
        emitBytes(OP_INVOKE, name);
        emitByte(argCount);
    } else {
        emitBytes(OP_GET_PROPERTY, name);
    }
}

static void __LIT(bool canAssign) {
    switch (digester.previous.type) {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NULL: emitByte(OP_NULL); break;
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        default: ;
    }
}

static void __GRP(bool canAssign) {
    expression();
    consume(TOKEN_GROUP_CLOSE, "Expect ')' after expression.");
}

static void __ARR(bool canAssign) {
    int length = 0;
    while (!check(TOKEN_ARRAY_CLOSE)) {
        expression(); 
        length++;
        if (check(TOKEN_COMMA))
            next();
        else
            break;          
    }
    consume(TOKEN_ARRAY_CLOSE, "expected ] after array definition");
    emitBytes(OP_ARRAY, makeConstant(NUMBER_VAL(length)));
}

static void __NUM(bool canAssign) {
    double value = strtod(digester.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void __OR(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    digestWeight(W_OR);
    patchJump(endJump);
}

static void __STR(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(digester.previous.start + 1,
                 digester.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    byte getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(current, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();

        emitBytes(setOp, (byte)arg);
    } else {

        emitBytes(getOp, (byte)arg);
    }
}

static void __REF(bool canAssign) {
    namedVariable(digester.previous, canAssign);
}

static Token syntheticToken(const char* text) {
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

static void __SUP(bool canAssign) {
    if (currentClass == NULL) {
        error("Can't use 'super' outside of a class.");
    } else if (!currentClass->inherited) {
        error("Can't use 'super' in a class with no superclass.");
    }

    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    byte name = identifierConstant(&digester.previous);

    namedVariable(syntheticToken("this"), false);

    if (match(TOKEN_GROUP_OPEN)) {
        byte argCount = argumentList();
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_SUPER_INVOKE, name);
        emitByte(argCount);
    } else {
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_GET_SUPER, name);
    }
}

static void __CUR(bool canAssign) {
    if (currentClass == NULL) {
        error("this only works inside a qlass context");
        return;
    }

    __REF(false);
}

static void __MOD(bool canAssign) {
    TokenType operatorType = digester.previous.type;
    digestWeight(W_UNARY);
    switch (operatorType) {
        case TOKEN_BANG: emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: ;
    }
}

WeightRule rules[] = {
    [TOKEN_GROUP_OPEN]      = { __GRP,  __CAL,  W_CALL },
    [TOKEN_GROUP_CLOSE]     = { NULL,   NULL,   W_NONE },
    [TOKEN_CONTEXT_OPEN]    = { NULL,   NULL,   W_NONE },
    [TOKEN_CONTEXT_CLOSE]   = { NULL,   NULL,   W_NONE },
    [TOKEN_ARRAY_OPEN]      = { __ARR,  NULL,   W_NONE },
    [TOKEN_ARRAY_CLOSE]     = { NULL,   NULL,   W_NONE },
    [TOKEN_COMMA]           = { NULL,   NULL,   W_NONE },
    [TOKEN_DOT]             = { NULL,   __DOT,  W_CALL },
    [TOKEN_MINUS]           = { __MOD,  __BIN,  W_TERM },
    [TOKEN_PLUS]            = { NULL,   __BIN,  W_TERM },
    [TOKEN_BITWISE_AND]     = { NULL,   __BIN,  W_TERM },
    [TOKEN_BITWISE_OR]      = { NULL,   __BIN,  W_TERM },
    [TOKEN_BITWISE_XOR]     = { NULL,   __BIN,  W_TERM },
    [TOKEN_BITWISE_NOT]     = { __MOD,  NULL,   W_TERM },
    [TOKEN_SEMICOLON]       = { NULL,   NULL,   W_NONE },
    [TOKEN_SLASH]           = { NULL,   __BIN,  W_FACTOR },
    [TOKEN_STAR]            = { NULL,   __BIN,  W_FACTOR },
    [TOKEN_BANG]            = { __MOD,  NULL,   W_NONE },
    [TOKEN_BANG_EQUAL]      = { NULL,   __BIN,  W_EQUALS },
    [TOKEN_EQUAL]           = { NULL,   NULL,   W_NONE },
    [TOKEN_EQUAL_EQUAL]     = { NULL,   __BIN,  W_EQUALS },
    [TOKEN_GREATER]         = { NULL,   __BIN,  W_COMPARE },
    [TOKEN_GREATER_EQUAL]   = { NULL,   __BIN,  W_COMPARE },
    [TOKEN_LESS]            = { NULL,   __BIN,  W_COMPARE },
    [TOKEN_LESS_EQUAL]      = { NULL,   __BIN,  W_COMPARE },
    [TOKEN_IDENTIFIER]      = { __REF,  NULL,   W_NONE },
    [TOKEN_STRING]          = { __STR,  NULL,   W_NONE },
    [TOKEN_NUMBER]          = { __NUM,  NULL,   W_NONE },
    [TOKEN_AND]             = { NULL,   __AND,  W_AND },
    [TOKEN_CLASS]           = { NULL,   NULL,   W_NONE },
    [TOKEN_ELSE]            = { NULL,   NULL,   W_NONE },
    [TOKEN_FALSE]           = { __LIT,  NULL,   W_NONE },
    [TOKEN_FOR]             = { NULL,   NULL,   W_NONE },
    [TOKEN_FUNCTION]        = { NULL,   NULL,   W_NONE },
    [TOKEN_IF]              = { NULL,   NULL,   W_NONE },
    [TOKEN_NULL]            = { __LIT,  NULL,   W_NONE },
    [TOKEN_OR]              = { NULL,   __OR,   W_OR },
    [TOKEN_PRINT]           = { NULL,   NULL,   W_NONE },
    [TOKEN_TYPEOF]          = { NULL,   NULL,   W_NONE },
    [TOKEN_RETURN]          = { NULL,   NULL,   W_NONE },
    [TOKEN_SUPER]           = { __SUP,  NULL,   W_NONE },
    [TOKEN_THIS]            = { __CUR,  NULL,   W_NONE },
    [TOKEN_TRUE]            = { __LIT,  NULL,   W_NONE },
    [TOKEN_VAR]             = { NULL,   NULL,   W_NONE },
    [TOKEN_WHILE]           = { NULL,   NULL,   W_NONE },
    [TOKEN_ERROR]           = { NULL,   NULL,   W_NONE },
    [TOKEN_EOF]             = { NULL,   NULL,   W_NONE }
};

static void digestWeight(Weight precedence) {
    next();
    WeightCallback prefixRule = getRule(digester.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("expecting expression");
        return;
    }

    bool canAssign = precedence <= W_ASSIGN;
    prefixRule(canAssign);

    while (precedence <= getRule(digester.current.type)->weight) {
        next();
        WeightCallback fn = getRule(digester.previous.type)->infix;
        fn(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static WeightRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    digestWeight(W_ASSIGN);
}

static void block() {
    while (!check(TOKEN_CONTEXT_CLOSE) && !check(TOKEN_EOF))
        declaration();
    consume(TOKEN_CONTEXT_CLOSE, "missing } after block.");
}

static void array(ValueType type) {

}

static void function(FunctionType type) {
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    consume(TOKEN_GROUP_OPEN, "expected ( after funq declaration");
    if (!check(TOKEN_GROUP_CLOSE)) {
        do {
            current->function->argc++;
            if (current->function->argc > 255) {
                errorAtCurrent("Can't have more than 255 parameters.");
            }
            byte constant = parseVariable("Expect parameter name.");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_GROUP_CLOSE, "Expect ')' after parameters.");
    consume(TOKEN_CONTEXT_OPEN, "Expect '{' before function body.");
    block();

    PtrFunq* function = endCompiler();

    emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++) {
        emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.upvalues[i].index);
    }
}

static void method() {
    consume(TOKEN_IDENTIFIER, "Expect method name.");
    byte constant = identifierConstant(&digester.previous);

    FunctionType type = F_INST;
    if (digester.previous.length == 4 &&
        memcmp(digester.previous.start, "init", 4) == 0) {
        type = F_INIT;
    }

    function(type);
    emitBytes(OP_METHOD, constant);
}

static void classDeclaration() {
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    Token className = digester.previous;
    byte nameConstant = identifierConstant(&digester.previous);
    declareVariable();

    emitBytes(OP_CLASS, nameConstant);
    defineVariable(nameConstant);

    ClassCompiler classCompiler;
    classCompiler.inherited = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;

    if (match(TOKEN_LESS)) {
        consume(TOKEN_IDENTIFIER, "missing inheriting qlass name");
        __REF(false);

        if (identifiersEqual(&className, &digester.previous)) {
            error("qlass can't inherit itself");
        }

        beginScope();
        addLocal(syntheticToken("super"));
        defineVariable(0);

        namedVariable(className, false);
        emitByte(OP_INHERIT);
        classCompiler.inherited = true;
    }

    namedVariable(className, false);
    consume(TOKEN_CONTEXT_OPEN, "Expect '{' before class body.");
    while (!check(TOKEN_CONTEXT_CLOSE) && !check(TOKEN_EOF)) {
        method();
    }
    consume(TOKEN_CONTEXT_CLOSE, "Expect ' }' after class body.");
    emitByte(OP_POP);

    if (classCompiler.inherited) {
        endScope();
    }

    currentClass = currentClass->enclosing;
}

static void funqDeclaration() {
    byte global = parseVariable("missing funqtion name");
    markInitialized();
    function(F_FUNC);
    defineVariable(global);
}

static void varDeclaration() {
    byte global = parseVariable("provide your variable with a name");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NULL);
    }
    if (digester.current.type != TOKEN_EOF)
        consume(TOKEN_SEMICOLON, "missing ; after variable declaration");

    defineVariable(global);
}

static void expressionStatement() {
    expression();
    if (digester.current.type != TOKEN_EOF)
        consume(TOKEN_SEMICOLON, "missing ; after expression");
    emitByte(OP_POP);
}

static void forStatement() {
    beginScope();
    consume(TOKEN_GROUP_OPEN, "Expect '(' after 'for'.");

    if (match(TOKEN_SEMICOLON)) {
        // No initializer.
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = currentSegment()->count;

    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();

        if (digester.current.type != TOKEN_EOF)
            consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }


    if (!match(TOKEN_GROUP_CLOSE)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentSegment()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_GROUP_CLOSE, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope();
}

static void ifStatement() {
    consume(TOKEN_GROUP_OPEN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_GROUP_CLOSE, "Expect ')' after condition."); // [paren]

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

static void printType() {
    expression();
    if (digester.current.type != TOKEN_EOF)
        consume(TOKEN_SEMICOLON, "missing ; after expression");
    emitByte(OP_TYPEOF);
}

static void printStatement() {
    expression();
    if (digester.current.type != TOKEN_EOF)
        consume(TOKEN_SEMICOLON, "missing ; after expression");
    emitByte(OP_PRINT);
}

static void returnStatement() {
    if (current->type == F_CODE) {
        error("Can't return from top-level code.");
    }

    if (match(TOKEN_SEMICOLON)) {
        emitReturn();
    } else {
        if (current->type == F_INIT) {
            error("Can't return a value from an initializer.");
        }

        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

static void whileStatement() {
    int loopStart = currentSegment()->count;
    consume(TOKEN_GROUP_OPEN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_GROUP_CLOSE, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

static void synchronize() {
    digester.panic = false;

    while (digester.current.type != TOKEN_EOF) {
        if (digester.previous.type == TOKEN_SEMICOLON) return;
        switch (digester.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUNCTION:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_TYPEOF:
            case TOKEN_RETURN:
                return;

            default:;
        }

        next();
    }
}

static void declaration() {
    if (match(TOKEN_CLASS)) {
        classDeclaration();
    } else if (match(TOKEN_FUNCTION)) {
        funqDeclaration();
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (digester.panic) synchronize();
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_TYPEOF)) {
        printType();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_RETURN)) {
        returnStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_CONTEXT_OPEN)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

PtrFunq* digest(const char* source) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler, F_CODE);

    digester.failed = false;
    digester.panic = false;

    next();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    PtrFunq* function = endCompiler();
    return digester.failed ? NULL : function;
}

void markCompilerRoots() {
    Compiler* compiler = current;
    while (compiler != NULL) {
        markObject((Ptr*)compiler->function);
        compiler = compiler->enclosing;
    }
}

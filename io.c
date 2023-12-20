#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "io.h"
#include "segment.h"
#include "debug.h"
#include "runner.h"
#include "string.h"

char* readFile(char* path) {
    FILE* file = fopen((const char*)path, "rb");
    if (file == NULL) {
        fprintf(stderr, "file not found or not readable \"%s\".\n", path);
        exit(E_ERR_IO_NOFILE);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "memory heap exceeded whilst reading \"%s\".\n", path);
        exit(E_ERR_SYS);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "file not readable \"%s\".\n", path);
        exit(E_ERR_IO_BADFILE);
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

void runFile(char* path, int flags) {
    char* source = readFile(path);
    int offset = doImports(source);
    InterpretResult result = interpret(path, source + offset);
    free(source);

    if (result == SQR_INTRP_ERROR_DIGEST) exit(E_ERR_SQR_DIGEST);
    if (result == SQR_INTRP_ERROR_RUNTIME) exit(E_ERR_SQR_RUNTIME);
}

int doImports(char* source) {
    InterpretResult r = SQR_INTRP_OK;
    int n = 0;
    do {
        char* c = (source + n);
        if (memcmp(c, "#import", 7) == 0) {
            n += 8;
            int length = getCharLength(c + n, '\n');
            char fileName[256];
            memcpy(fileName, c + n, length);
            char* src = readFile(fileName);
            r = (int)interpret(fileName, src);
            n += length + 1;
        }
        else break;
    } while (r == SQR_INTRP_OK);
    return n;
}
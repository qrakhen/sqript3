#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "segment.h"
#include "digest.h"
#include "log.h"
#include "runtime.h"

static void runConsole() {
    char line[1024];
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        interpret(line);
    }
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);
    if (result == INTERPRET_COMPILE_ERROR) exit(E_ERR_SQR_COMPILE);
    if (result == INTERPRET_RUNTIME_ERROR) exit(E_ERR_SQR_RUNTIME);
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "could not find file: \"%s\".\n", path);
        exit(E_ERR_IO_NOFILE);
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char* buffer = (char*)malloc(fileSize + 1);
    if (bytesRead < fileSize) {
        fprintf(stderr, "file seems to be corrupt: \"%s\".\n", path);
        exit(E_ERR_IO_BADFILE);
    }
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, const char* argv[])
{
    initRuntime();

    if (argc == 1) {
        runConsole();
    } else if (argc > 1) {
        runFile(argvs[1]);
    }

    clearRuntime();
    return 0;
}

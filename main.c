#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "segment.h"
#include "debug.h"
#include "runner.h"
#include "console.h"

#define __C_PREFIX_INPUT " <: "
#define __C_PREFIT_OUTPUT " :> "

static void runConsole() {
    char line[1024];
    for (;;) {
        printf(__C_PREFIX_INPUT);
        if (!fgets(line, sizeof(line), stdin)) {
            consoleWriteLine("");
            break;
        }
        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
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

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == SQR_INTRP_ERROR_DIGEST) exit(E_ERR_SQR_COMPILE);
    if (result == SQR_INTRP_ERROR_RUNTIME) exit(E_ERR_SQR_RUNTIME);
}

int main(int argc, const char* argv[]) {
    consoleInit();
    initRunner();
    if (argc == 1) {
        runConsole();
    } else if (argc >= 2) {
        if (argc > 3) {
            printf("options");
        }
        runFile(argv[1]);
    }
    freeRunner();
    return 0;
}

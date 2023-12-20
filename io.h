#ifndef sqript_io_h
#define sqript_io_h

char* readFile(char* path);
void runFile(const char* path, int flags);
int doImports(char* source);

#endif
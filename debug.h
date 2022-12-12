#ifndef sqript_debug_h
#define sqript_debug_h

void DBG_printBytes(void* v, tSize, length, size_t size) {
    char* c = (char*) v;
    for (int i = 0; i < length * tSize; i++)
        printf("%hhx ");

}

#endif

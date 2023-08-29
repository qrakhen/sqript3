#ifndef sqript_options_h
#define sqript_options_h

typedef struct {
    const char* key;
    char c;
    const char* description;
} LaunchOptionInfo;

typedef struct {
    LaunchOptionInfo* info;
    int value;
    char* strValue;
} LaunchOption;

LaunchOptionInfo __LAUNCH_OPTION_INFOS[32];
LaunchOption __ARGS[64];
int __ARGC;

LaunchOption* __GET_ARG(const char* key) {
    int i = 0;
    do {
        LaunchOption o = *(__ARGS + i);
        if (o.info == NULL)
            return NULL;
        if (strcmp(key, o.info->key) == 0)
            return &o;
    } while (i++ < __ARGC);
    return NULL;
}

static void __set(int index, const char* key, char c, const char* description) {
    LaunchOptionInfo* info = (__LAUNCH_OPTION_INFOS + index);
    info->key = key;
    info->c = c;
    info->description = description;
}

LaunchOptionInfo* getLaunchOptionInfos() {
    __set(0, "file", "f", "file to execute");
    __set(1, "log-level", "l", "log level (0-2)");
    __set(2, "keep-alive", "k", "whether sqript should terminate after execution");
    __set(3, "cache", "c", "whether bytecodes should be cached for re-use");
}

static LaunchOption parseLaunchOption(const char* str) {
    LaunchOption option;
    option.info = NULL;
    option.value = -1;
    option.strValue = NULL;
    char* c = str;
    int start = 0;
    int length = 0;
    do {
        if (*c == '-') {
            start++;
            continue;
        }
        else break;     
    } while (*(c++) != '\0'); 
    do {
        if (*c != '=') {
            length++;
            continue;
        }
        else break;
    } while (*(c++) != '\0');

    if (start == 0) {
        option.info = &__LAUNCH_OPTION_INFOS[0];
        option.value = 1;
        option.strValue = str;
        return option;
    }
    
    for (int i = 0; i < 4; i++) {
        LaunchOptionInfo* info = &__LAUNCH_OPTION_INFOS[i];
        if ((start == 1 && *c == info->c) ||
            (start == 2 && memcmp(str + start, info->key, length) == 0)) {
            option.info = info;
            option.value = atoi(str + start + length + 1);
            return option;
        }
    }

    return option;
}

LaunchOption* parseLaunchOptions(int argc, const char* argv[]) {
    getLaunchOptionInfos();
    int n = 0;
    for (int i = 1; i < argc; i++) {
        LaunchOption option = parseLaunchOption(argv[i]);
        if (option.info != NULL)
            __ARGS[n++] = option;
        else
            printf("Unkown launch option: %s\n", argv[i]);
    }
    __ARGC = argc;
    return __ARGS;
}

#endif
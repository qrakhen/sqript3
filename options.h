#ifndef sqript_options_h
#define sqript_options_h

#define LAUNCH_OPTION_NO_VALUE 0xffffffff

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

int __optionIndex = 0;
static void __set(char* key, char c, const char* description) {
    LaunchOptionInfo* info = (__LAUNCH_OPTION_INFOS + __optionIndex++);
    info->key = key;
    info->c = c;
    info->description = description;
}

LaunchOptionInfo* getLaunchOptionInfos() {
    __set("config",         "c", "config to be used (*.qfc) [default=]");
    __set("out",            "o", "compile target [default='./out/']");
    __set("source",         "s", "source *.prq file to build, alternatively provide the root folder [default='./']");
    __set("run",            "r", "file to execute (compiles and directly executes) [default=]");
    __set("log-level",      "l", "log level (0-5) [default=0]");
    __set("log-file",       "w", "file to write logs to. always verbose. [default=]");
    __set("no-null",        "n", "null references are forbidden [default=0]");
    __set("no-overwrite",   "r", "prohibit overwriting existing identifiers [default=0]");
    __set("no-undeclared",  "u", "prohibit using undeclared values [default=1]");
    __set("keep-alive",     "k", "whether sqript should terminate after execution [default=1]");
    __set("cache",          "h", "whether bytecodes should be cached for re-use [default=0]");
}

static LaunchOption parseLaunchOption(const char* str) {
    LaunchOption option;
    option.info = NULL;
    option.value = LAUNCH_OPTION_NO_VALUE;
    option.strValue = NULL;
    char* c = str;
    int start = 0;
    int length = 0;
    int _length = 0;
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
        } else {
            do {
                if (*c != '\0') {
                    _length++;
                    continue;
                }
                else break;
            } while (*(c++) != '\0');
        }
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
            if (_length == 0)
                option.value = 1;
            else
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
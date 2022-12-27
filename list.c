#include <stdlib.h>

#include "list.h";

static void freeNode(Node* node) {
    free(node);
}
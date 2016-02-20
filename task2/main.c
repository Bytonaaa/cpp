#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    
};

int main(int argc, const char *argv[]) {
    FILE *fp = fopen(argv[1], "r+");
    if (fp == NULL) {
        fp = fopen(argv[1], "w+");
        if (fp == NULL) {
            printf("Can't create a file");
            exit(1);
        }
    }

    for(;;) {
        char cmd[1024];
        if (!strcmp("exit", cmd)) {
            break;
        } else if (!strcmp("create", cmd)) {

        }
    }
}
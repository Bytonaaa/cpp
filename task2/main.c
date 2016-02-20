#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
    FILE *fp = fopen(argv[1], "r+");
    if (fp == NULL) {
        fp = fopen(argv[1], "w+");
        if (fp == NULL) {
            printf("Can't create a file");
            exit(1);
        }
    }


}
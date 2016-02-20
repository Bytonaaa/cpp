#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
    int n, *table;
    scanf("%d", &n);

    table = malloc(n * n * sizeof(int));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            table[i * n + j] = (i + 1) * (j + 1);
        }
    }

    for (; ;) {
        int x1, x2, y1, y2;
        scanf("%d", &x1);
        if (x1 == 0) {
            break;
        }

        scanf("%d %d %d", &y1, &x2, &y2);

        for (int j = y1 - 1; j < y2; j++) {
            for (int i = x1 - 1; i < x2; i++) {
                printf("%d ", table[i * n + j]);
            }
            putchar('\n');
        }
    }

    free(table);
}

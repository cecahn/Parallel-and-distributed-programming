#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 10000
#define COLS 10000
#define MIN_VAL 1000000000LL
#define MAX_VAL 9999999999LL

long long get_random_large() {
    return MIN_VAL + (rand() % (MAX_VAL - MIN_VAL + 1));
}

int main() {
    FILE *file = fopen("large_matrix10000.txt", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    srand((unsigned int)time(NULL));

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            long long value = get_random_large();
            fprintf(file, "%lld", value);
            if (j < COLS - 1) {
                fputc(' ', file);
            }
        }
        fputc('\n', file);
    }

    fclose(file);
    printf("Matrix written to large_matrix.txt\n");
    return 0;
}


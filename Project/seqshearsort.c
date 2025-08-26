#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
//#include "sorthelp.h"
//#include <mpi.h>

int sorted_ascending(int *elements, int n, int size)
{
    for (int i = 0; i < n - 1; i++)
    if(n % size != 0)
    {
        if (elements[i] > elements[i + 1])
        {
            return 0;
        }
            
    }
        
    return 1;
}

int sorted_descending(int *elements, int n, int size)
{
    for (int i = 0; i < n - 1; i++)
        if(n % size != 0)
        {
            if (elements[i] < elements[i + 1])
            {
                return 0;
            }
            
        }
        
    return 1;
}

int *seqshearsort(int *matrix, int size){
    for (int i = 0; i < size; i++){
        for (int r = 0; r < size; r++ ){
                for(int j = 0; j< size; j++){
                    if(r!=j && r<j){
                        if(matrix[i*size + r]> matrix[i * size +j]){
                            int temp = matrix[i*size + r];
                            matrix[i*size + r] = matrix[i*size + j];
                            matrix[i * size + j] = temp;
                        }
                    }
                }
            }
    }

    //printMatrix(matrix, size, size);

    for (int i = 0; i < size; i++){
        if(i % 2 != 0)
        {
            for (int r = 0; r < size; r++ )
            {
                for(int j = 0; j< size; j++){
                    if(r!=j && r<j){
                        if(matrix[i*size + r]< matrix[i * size +j]){
                            int temp = matrix[i*size + r];
                            matrix[i*size + r] = matrix[i*size + j];
                            matrix[i * size + j] = temp;
                        }
                    }
                }
            }
            
        }
    }

    //printMatrix(matrix, size, size);

    for (int k = 0; k < size; k++){
        
            for(int r = 0; r < size; r ++){
                for(int j = 0; j < size; j++)
                {
                    if(r != j && r < j)
                    {
                        if(matrix[r*size + k]>matrix[j*size + k])
                            {
                                //switch places
                                int temp = matrix[r*size + k];
                                matrix[r*size + k] = matrix[j*size + k];
                                matrix[j*size + k] = temp;
                            }
                    }
                    
                }
                
            }
        
    }

    //printMatrix(matrix, size, size);

    return matrix;

}

int** allocateMatrix(int rows, int cols) {
    int** matrix = malloc(rows * sizeof(int*));
    if (matrix == NULL) {
        perror("Failed to allocate memory for rows");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; ++i) {
        matrix[i] = malloc(cols * sizeof(int));
        if (matrix[i] == NULL) {
            perror("Failed to allocate memory for columns");
            exit(EXIT_FAILURE);
        }
    }

    return matrix;
}



int read_file(char *file_name, int **elements){
    FILE *f = fopen(file_name, "r");

    if (!f)
    {
        return -1;
    }

    int n;

    if (fscanf(f, "%d", &n) != 1)
    {
        fclose(f);
        return -1;
    }

    *elements = malloc(n * n * sizeof(int));

    for (int i = 0; i < n*n; i++)
    {
        if (fscanf(f, "%d", *elements + i) != 1)
        {
            fclose(f);
            free(elements);
            return -1;
        }
    }

    fclose(f);
    return n;
}

int printvalidate(int *elements, int size){
    for(int i = 0; i < size; i ++){

        for(int j = 0; j < size; i++){
            if(((size*i)+j) % 2 == 0){
                if (sorted_ascending(elements, ((size*i)+j), size) == 0)
                {
                     return 0; 
                }
               
            }
            else if(((size*i)+j) % 2 != 0){
                if (sorted_descending(elements, ((size*i)+j), size) == 0)
                {
                    return 0; 
                }
                
            }
        }

    }

    for (int i = 0; i < size; i++) {
        printf( "%d ", elements[i]);
        if(i % size){
            printf("\n");
        }
    }
}

void printmatrix(int * matrix, int size){
    for (int i = 0; i < size*size; i++) {
        if(i != 0 && i % size == 0){
            printf("\n");
        }
        printf( "%d ", matrix[i]);
        
    }

    printf("\n \n");
}


int write_to_file(char *outfile, int size, int* matrix){
    FILE *f = fopen(outfile, "w");
    if (!f)
    {
        return -2;
    }

    for (int i = 0; i < size*size; i++) {
        if(i != 0 && i % size == 0){
            fprintf(f, "\n");
        }
        fprintf(f, "%d ", matrix[i]);
    }

    fclose(f);
}

void freeMatrix(int** matrix, int rows) {
    for (int i = 0; i < rows; ++i)
        free(matrix[i]);
    free(matrix);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* infile = argv[1];
    char* outfile = argv[2];

    int *matrix = NULL;
    int size = read_file(infile, &matrix);

    if (size == -1) {
        fprintf(stderr, "Failed to read matrix from file.\n");
        return EXIT_FAILURE;
    }

    int log = 0;
    while ((1 << log) < size) log++;

    printf("log = %d\n", log);

    // Optional: print input matrix
    // printmatrix(matrix, size);

    clock_t start = clock(); // Start timing

    for (int i = 0; i < log + 1; i++) {
        seqshearsort(matrix, size);
    }

    clock_t end = clock(); // End timing

    double elapsed_secs = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken to sort: %.6f seconds\n", elapsed_secs);

    // printmatrix(matrix, size);

    write_to_file(outfile, size, matrix);

    free(matrix);
    return 0;
}


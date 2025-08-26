#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <math.h>

#define ASCENDING true
#define DESCENDING false
#define ROOT 0

//help function for sorting in ascending order
int compareasc(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

//help function for sorting in descending order
int comparedesc(const void *a, const void *b) {
    return (*(int*)b - *(int*)a);
}

//sorts a row of the matrix using quick sort
void sortrow(int *row, int size, bool ascending) {
    qsort(row, size, sizeof(int), ascending ? compareasc : comparedesc);
}

//reads the input textfile containing the matrix
int readfile(char *file_name, int **elements){
    FILE *f = fopen(file_name, "r");
    if (!f){
        return -1;
    }
    int n;
    if (fscanf(f, "%d", &n) != 1){
        fclose(f);
        return -1;
    }
    *elements = malloc(n * n * sizeof(int));
    for (int i = 0; i < n*n; i++){
        if (fscanf(f, "%d", *elements + i) != 1)
        {
            fclose(f);
            free(*elements);
            return -1;
        }
    }
    fclose(f);
    return n;
}

//writes to output text file
int write_to_file(char *outfile, int size, int* matrix){
    FILE *f = fopen(outfile, "w");
    if (!f){
        return -2;
    }
    for (int i = 0; i < size*size; i++) {
        if(i != 0 && i % size == 0){
            fprintf(f, "\n");
        }
        fprintf(f, "%d ", matrix[i]);
    }
    fclose(f);
    return 0;
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

//checks if the matrix is sorted correctly
bool isshearsorted(int *matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size - 1; j++) {
            if (i % 2 == 0) { 
                if (matrix[i * size + j] > matrix[i * size + j + 1])
                    return false;
            } else { 
                if (matrix[i * size + j] < matrix[i * size + j + 1])
                    return false;
            }
        }
    }
    for (int j = 0; j < size; j++) {
        for (int i = 0; i < size - 1; i++) {
            if (matrix[i * size + j] > matrix[(i + 1) * size + j])
                return false;
        }
    }
    return true;
}


//transposes the full matrix in parallel
void transpose_distributed(
    int *local_matrix, int **newlocalmatrix,
    int *rowsperproc, int *rowdispls,
    int size, int rank, int numprocesses, int* sendcounts, int *recvcounts, int* sdispls, int* rdispls) {

    //the number of rows the current process is assigned
    int localrows = rowsperproc[rank];
    
    //calculate the number of values each process should send and receive
    for (int i = 0; i < numprocesses; i++) {
        sendcounts[i] = localrows * rowsperproc[i];
        recvcounts[i] = localrows * rowsperproc[i];
    }

    //calculate indexes for each process 
    sdispls[0] = rdispls[0] = 0;
    for (int i = 1; i < numprocesses; i++) {
        sdispls[i] = sdispls[i - 1] + sendcounts[i - 1];
        rdispls[i] = rdispls[i - 1] + recvcounts[i - 1];
    }

    //calculate total values that should be sent and received by the local process
    int total_send = sdispls[numprocesses - 1] + sendcounts[numprocesses - 1];
    int total_recv = rdispls[numprocesses - 1] + recvcounts[numprocesses - 1];

    
    int *sendbuf = malloc(total_send * sizeof(int));
    if (!sendbuf) {
        fprintf(stderr, "sendbuf malloc failed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int *recvbuf = malloc(total_recv * sizeof(int));
    //prepare send buf
    int idx = 0;
    for (int p = 0; p < numprocesses; p++) {
        //used to find the right values to send to each process
        int col_start = rowdispls[p];
        int col_count = rowsperproc[p];
        for (int i = 0; i < localrows; i++) {
            for (int j = 0; j < col_count; j++) {
                sendbuf[idx++] = local_matrix[i * size + col_start + j];
            }
        }
    }

    //send values in sendbuf to respective process receive values in recvbuf
    //each process p have the number of values to send and recieve in sendcounts[p] and recvcounts[p]
    //each process p have its index for send and receive buffer in sdispls[p] and rdispls[p]
    MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_INT,
                  recvbuf, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD);
    
    //order values in correct transposed order from receivebuf
    int *transposed = malloc(rowsperproc[rank] * size * sizeof(int));
    idx = 0; 
    for (int p = 0; p < numprocesses; p++) {
        //number of rows per process== number of columns per process
        //calculate from how many rows the process received values
        int rowsfromp = rowsperproc[p]; 
        for (int i = 0; i < rowsfromp; i++) {
            for (int j = 0; j < rowsperproc[rank]; j++) {
                transposed[j * size + rowdispls[p] + i] = recvbuf[idx++];
            }
        }
    }
    free(sendbuf);
    free(recvbuf);

    *newlocalmatrix = transposed;
    
}


int main(int argc, char*argv[]){

    MPI_Init(&argc, &argv);

    int myid, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile> \n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    char *infile = argv[1]; 
    char *outfile = argv[2]; 
    int *matrix = NULL;
    int size;
    double start = 0; 

    //Read file
    if (myid == ROOT)
    {
        size = readfile(infile, &matrix);

        if (size == -1){
            fprintf(stderr, "Error reading input file\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            MPI_Finalize();
            return 1;
        }
        //start timing
        start = MPI_Wtime();
        //printmatrix(matrix, size);
    }

    //broadcast size of matrix
    MPI_Bcast(&size, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    int ITERATIONS = (int)(log2(size)) + 1;

    int baserows = size/ p;
    int rest = size % p;

    int offset = 0;
    int *rowsperproc = malloc(p * sizeof(int));
    int *rowdispls = malloc(p * sizeof(int));
    //for transpose
    int *tsendcounts = malloc(p * sizeof(int));
    int *trecvcounts = malloc(p * sizeof(int));
    int *tsdispls = malloc(p * sizeof(int));
    int *trdispls = malloc(p * sizeof(int));
    for (int i = 0; i < p; i++) {
        //evenly distribute rows between processes
        if(rest>i)
        {
            rowsperproc[i] = baserows + 1; 
        }
        else{
            rowsperproc[i] = baserows; 
        }
        //calculate at which row number each process begins
        rowdispls[i] = offset;
        offset += rowsperproc[i];
    }

    int numrows = rowsperproc[myid];
    int *localmatrix = malloc(numrows * size * sizeof(int));

    int *sendcounts = malloc(p * sizeof(int));
    int *displs = malloc(p * sizeof(int));
    //calculate how many values should be sent to each process
    for (int i = 0; i < p; i++) {
        sendcounts[i] = rowsperproc[i] * size;
        displs[i] = rowdispls[i] * size;
    }

    //distribute the matrix between the processes
    MPI_Scatterv(
            matrix, sendcounts, displs, MPI_INT,
            localmatrix, sendcounts[myid], MPI_INT,
            ROOT, MPI_COMM_WORLD
        );

    for(int iter = 0; iter<ITERATIONS; iter ++){

        //sort rows
        for(int i = 0; i < numrows; i ++){
            int globalrowid = rowdispls[myid]+i;
            bool order;
            if(globalrowid%2 == 0){
                order = ASCENDING;
            } 
            else{
                order = DESCENDING; 
            }
            sortrow(&localmatrix[i*size], size, order);

            
            
        }

        //transpose
        int *newlocal = NULL;
            transpose_distributed(localmatrix, &newlocal,
                              rowsperproc, rowdispls,
                              size, myid, p, tsendcounts, trecvcounts, tsdispls, trdispls); 

        //each process needs a new local matrix as the other one is split up and reordered (to avoid segfault)                     
        free(localmatrix);
        localmatrix = newlocal;

        //sort columns    
        for(int i = 0; i < numrows; i ++)
        {
            //columns are now rows
            sortrow(&localmatrix[i*size], size, ASCENDING);
        }

        //transpose back
        newlocal = NULL;
        transpose_distributed(localmatrix, &newlocal,
                              rowsperproc, rowdispls,
                              size, myid, p, tsendcounts, trecvcounts, tsdispls, trdispls); 
                              
        free(localmatrix);
        localmatrix = newlocal;

       
        
    }
    //Gather into the final matrix
    MPI_Gatherv(localmatrix, sendcounts[myid], MPI_INT,
                myid == ROOT ? matrix : NULL, sendcounts, displs, MPI_INT,
                ROOT, MPI_COMM_WORLD);


    if(myid == ROOT){
        //stop timing
        double end = MPI_Wtime();
        printf("time: %f\n", end - start);
        write_to_file(outfile, size, matrix);
        //print and check matrix if it is smaller than 20x20
        if(size<=20)
        {
            printmatrix(matrix, size); 
            if(isshearsorted(matrix, size)){
                printf("sorted correctly \n");
            }
        }
        
        free(matrix);
    }
    free(sendcounts);
    free(displs);
    free(rowdispls);
    free(rowsperproc);
    free(localmatrix);
    free(tsendcounts);
    free(trecvcounts);
    free(tsdispls);
    free(trdispls);
    MPI_Finalize();
    return 0;

}
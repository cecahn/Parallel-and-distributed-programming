#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "quicksort.h"
#include "pivot.h"

int check_and_print(int *elements, int n, char *file_name)
{
    if (!sorted_ascending(elements, n))
    {
        for (int i = 0; i < n - 1; i++)
        {
            if (elements[i] > elements[i + 1])
            {
                fprintf(stderr, "Error: elements[%d]=%d > elements[%d]=%d\n", i, elements[i], i + 1, elements[i + 1]);
                break;
            }
        }
    }
    FILE *f = fopen(file_name, "w");
    if (!f)
    {
        return -2;
    }

    for (int i = 0; i < n; i++) {
        fprintf(f, "%d ", elements[i]);
    }

    fclose(f);
    return 0;
}

int distribute_from_root(int *all_elements, int n, int **my_elements) {
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int base = n / size;
    int rem = n % size;
    
    int *sendcounts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        sendcounts[i] = base + (i < rem ? 1 : 0);
        displs[i] = (i==0?0:displs[i-1] + sendcounts[i-1]);
    }
    
    *my_elements = malloc(sendcounts[rank] * sizeof(int));
    
    MPI_Scatterv(all_elements, sendcounts, displs, MPI_INT,
                 *my_elements, sendcounts[rank], MPI_INT,
                 ROOT, MPI_COMM_WORLD);

    int local_n = sendcounts[rank];
    free(sendcounts);
    free(displs);
    return local_n;
}

void gather_on_root(int *all_elements, int *my_elements, int local_n)
{
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int *recvcounts = NULL;
    int *displs = NULL;
    if (rank == ROOT)
    {
        recvcounts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
    }

    MPI_Gather(&local_n, 1, MPI_INT, recvcounts, 1, MPI_INT,
               ROOT, MPI_COMM_WORLD);

    if (rank == ROOT)
    {
        displs[0] = 0;
        for (int i = 1; i < size; i++)
        {
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }

    MPI_Gatherv(my_elements, local_n, MPI_INT,
                all_elements, recvcounts, displs, MPI_INT,
                ROOT, MPI_COMM_WORLD);

    if (rank == ROOT)
    {
        free(recvcounts);
        free(displs);
    }
}

void merge_ascending(int *v1, int n1, int *v2, int n2, int *result)
{
    int i = 0, j = 0, k = 0;
    while (i < n1 && j < n2)
    {
        if (v1[i] <= v2[j])
            result[k++] = v1[i++];
        else
            result[k++] = v2[j++];
    }
    while (i < n1)
        result[k++] = v1[i++];
    while (j < n2)
        result[k++] = v2[j++];
}

int read_input(char *file_name, int **elements)
{
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

    *elements = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++)
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

int sorted_ascending(int *elements, int n)
{
    for (int i = 0; i < n - 1; i++)
        if (elements[i] > elements[i + 1])
            return 0;
    return 1;
}

int global_sort(int **elements, int n, MPI_Comm comm, int pivot_strategy)
{
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    int local_n = n;
    int *local = *elements;
    
    int group_size = size;

    MPI_Comm local_comm;
    MPI_Comm_dup(comm, &local_comm);

    while (group_size > 1)
    {
        // select pivot
        int pivot = select_pivot(pivot_strategy, local, local_n, local_comm);

        int idx = get_larger_index(local, local_n, pivot); // Index of first element larger than pivot
        int low_count = idx;
        int high_count = local_n - idx;
    

        int *low = malloc(low_count * sizeof(int));
        int *high = malloc(high_count * sizeof(int));
        
        memcpy(low, local, low_count * sizeof(int));
        memcpy(high, local + idx, high_count * sizeof(int));
        
        // split communicator
        int color, group_rank;
        MPI_Comm_rank(local_comm, &group_rank);

        bool is_low = (group_rank < group_size / 2);
        color = (is_low) ? 0 : 1;
        int partner = (is_low) ? group_rank + group_size / 2 : group_rank - group_size / 2;

        // exchange counts
        int send_count = (color == 0 ? high_count : low_count);
        int recv_count;
        MPI_Sendrecv(&send_count, 1, MPI_INT, partner, 0,
                     &recv_count, 1, MPI_INT, partner, 0,
                     local_comm, MPI_STATUS_IGNORE);
        
        int *send_buf = (color == 0 ? high : low);
        int *recv_buf = malloc(recv_count * sizeof(int));

        MPI_Sendrecv(send_buf, send_count, MPI_INT, partner, 1,
                     recv_buf, recv_count, MPI_INT, partner, 1,
                     local_comm, MPI_STATUS_IGNORE);
        
        // reallocate and merge
        free(local);
        if (color == 0)
        {
            local_n = low_count + recv_count;
            local = malloc(local_n * sizeof(int));
            merge_ascending(low, low_count, recv_buf, recv_count, local);
        }
        else
        {
            local_n = high_count + recv_count;
            local = malloc(local_n * sizeof(int));
            merge_ascending(recv_buf, recv_count, high, high_count, local);
        }

        free(low);
        free(high);
        free(recv_buf);

        MPI_Comm new_comm;
        MPI_Comm_split(local_comm, color, group_rank, &new_comm);
        MPI_Comm_free(&local_comm);
        
        local_comm = new_comm;
        group_size /= 2;
    }
    *elements = local;
    MPI_Comm_free(&local_comm);
    return local_n;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile> <pivot_strategy>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    char *infile = argv[1];
    char *outfile = argv[2];
    int strategy = atoi(argv[3]);

    int *all = NULL; // all elements on root

    int n;
    double t0 = 0;
    if (rank == ROOT)
    {
        // printf("strategy: %d\n", strategy);
        n = read_input(infile, &all);
        // printf("n: %d\n", n);

        if (n == -1)
        {
            fprintf(stderr, "Error reading input file\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            MPI_Finalize();
            return 1;
        }
        t0 = MPI_Wtime();
    }
    // broadcast n to all processes
    MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    int *local; // local elements for each process
    int local_n = distribute_from_root(all, n, &local); // scatter to all processes

    // local sort
    qsort(local, local_n, sizeof(int), compare);

    int new_n = global_sort(&local, local_n, MPI_COMM_WORLD, strategy);
    int *result = NULL;

    if (rank == ROOT)
    {
        result = malloc(n * sizeof(int));
    }

    gather_on_root(result, local, new_n);

    if (rank == ROOT)
    {
        double t1 = MPI_Wtime();
        printf("%f\n", t1 - t0);
        check_and_print(result, n, outfile);

        free(result);
        free(all);
    }

    free(local);
    MPI_Finalize();
    return 0;
}

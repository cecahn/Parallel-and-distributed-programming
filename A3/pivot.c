// File: pivot.c
#include "pivot.h"
#include <stdlib.h>
#include <mpi.h>

int compare(const void *v1, const void *v2) {
    int a = *(const int*)v1;
    int b = *(const int*)v2;
    return (a > b) - (a < b);
}

int get_larger_index(int *elements, int n, int val) {
    int i;
    for (i = 0; i < n; i++) {
        if (elements[i] > val) return i;
    }
    return n;
}

int get_median(int *elements, int n) {
    if (n % 2 != 0) {
        return elements[n / 2];  // Odd: middle element
    } else {
        return (elements[(n-1)/2] + elements[n/2]) / 2.0;  // Even: average of two middle elements
    }
}

static int broadcast_pivot(int pivot, MPI_Comm comm) {
    MPI_Bcast(&pivot, 1, MPI_INT, ROOT, comm);
    return pivot;
}

int select_pivot_median_root(int *elements, int n, MPI_Comm communicator) {
    int pivot = -1;
    int rank;
    MPI_Comm_rank(communicator, &rank);
    if (rank == ROOT) {
        pivot = get_median(elements, n);
    }
    return broadcast_pivot(pivot, communicator);
}

int select_pivot_mean_median(int *elements, int n, MPI_Comm communicator) {
    int rank, size;
    MPI_Comm_rank(communicator, &rank);
    MPI_Comm_size(communicator, &size);
    int local_med = get_median(elements, n);
    int *medians = NULL;
    double mean = 0;
    if (rank == ROOT) {
        medians = malloc(size * sizeof(int));
    }
    MPI_Gather(&local_med, 1, MPI_INT, medians, 1, MPI_INT, ROOT, communicator);
    if (rank == ROOT) {
        long sum = 0;
        for (int i = 0; i < size; i++) sum += medians[i];
        mean = sum / (double)size;
        free(medians);
    }
    // broadcast mean as int pivot
    int pivot = (int)mean;
    return broadcast_pivot(pivot, communicator);
}

int select_pivot_median_median(int *elements, int n, MPI_Comm communicator) {
    int rank, size;
    MPI_Comm_rank(communicator, &rank);
    MPI_Comm_size(communicator, &size);
    int local_med = get_median(elements, n);
    int *medians = NULL;

    if (rank == ROOT) {
        medians = malloc(size * sizeof(int));
    }
    
    MPI_Gather(&local_med, 1, MPI_INT, medians, 1, MPI_INT, ROOT, communicator);
    int pivot = -1;
    if (rank == ROOT) {
        qsort(medians, size, sizeof(int), compare);
        pivot = get_median(medians, size);
        free(medians);
    }
    return broadcast_pivot(pivot, communicator);
}

int select_pivot(int pivot_strategy, int *elements, int n, MPI_Comm communicator) {
    switch (pivot_strategy) {
        case MEAN_MEDIAN:
            return select_pivot_mean_median(elements, n, communicator);
        case MEDIAN_MEDIAN:
            return select_pivot_median_median(elements, n, communicator);
        case MEDIAN_ROOT:
        default:
            return select_pivot_median_root(elements, n, communicator);
    }
}
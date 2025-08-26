#include "stencil.h"

static int find_start(int n, int p, int k)
{
    int q = n / p;
    int rest = n % p;

    if (k < rest)
    {
        return k * (q + 1);
    }
    return k * q + rest;
}

static int find_stop(int n, int p, int k)
{
    return find_start(n, p, k + 1) - 1;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
	
    int myid, p;
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	// printf("Process %d of %d\n", myid, p);

	if (4 != argc) {
		printf("Usage: stencil input_file output_file number_of_applications\n");
		return 1;
	}

	char *input_name = argv[1];
	char *output_name = argv[2];
	int num_steps = atoi(argv[3]);

	// Read input file
	double *input = NULL;
	double *global_output = NULL;
	int num_values;
	if (myid == 0) {
		num_values = read_input(input_name, &input);
		global_output = malloc(num_values * sizeof(double));
	}

	MPI_Bcast(&num_values, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Stencil values
	double h = 2.0*PI/num_values;
	const int STENCIL_WIDTH = 5;
	const int EXTENT = STENCIL_WIDTH/2;
	const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};

	// Dela upp input i bitar och scatter till alla processer
	int chunk_size = num_values / p;
	int chunk_halo_size = chunk_size + 2 * EXTENT;
	double *input_chunk = malloc(chunk_halo_size * sizeof(double));
	
	MPI_Scatter(input, chunk_size, MPI_DOUBLE,
				&input_chunk[EXTENT], chunk_size, MPI_DOUBLE,
				0, MPI_COMM_WORLD);

	// Allocate data for result
	double *output_chunk = malloc(chunk_halo_size * sizeof(double));

	// Start timer
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

	int left = (myid - 1 + p) % p;
    int right = (myid + 1) % p;

	for (int s = 0; s < num_steps; s++) {
		// Exchange halo regions
		// Send leftmost EXTENT from local_in[EXTENT] to left, receive from right into end halo
		MPI_Sendrecv(
			&input_chunk[EXTENT], EXTENT, MPI_DOUBLE, left, 0,
			&input_chunk[chunk_size + EXTENT], EXTENT, MPI_DOUBLE, right, 0,
			MPI_COMM_WORLD, MPI_STATUS_IGNORE);


		// Send rightmost EXTENT from local_in[EXTENT+local_n-EXTENT] to right, receive from left into start halo
		MPI_Sendrecv(
			&input_chunk[chunk_size], EXTENT, MPI_DOUBLE, right, 1,
			&input_chunk[0], EXTENT, MPI_DOUBLE, left, 1,
			MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Apply stencil on local data
		for (int i = EXTENT; i < chunk_size + EXTENT; i++) {
			double res = 0.0;
			for (int j = 0; j < STENCIL_WIDTH; j++) {
				res += STENCIL[j] * input_chunk[i - EXTENT + j];
			}
			output_chunk[i] = res;
		}

		// Swap input and output
		double *tmp = input_chunk;
		input_chunk = output_chunk;
		output_chunk = tmp;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	double local_time = MPI_Wtime() - start;
	double max_time;
	MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (myid == 0) {
		printf("time: %f\n", max_time);
	}

	MPI_Gather(
		&input_chunk[EXTENT], chunk_size, MPI_DOUBLE,
		global_output,     chunk_size, MPI_DOUBLE,
		0, MPI_COMM_WORLD);
		
#ifdef PRODUCE_OUTPUT_FILE
		if (myid == 0) {
			if (write_output(output_name, global_output, num_values) != 0) {
				MPI_Abort(MPI_COMM_WORLD, 5);
			}
			free(input);
		}
#endif


	// Clean up
	free(output_chunk);
	// free(global_output);

	MPI_Finalize();

	return 0;
}

// Dont change
int read_input(const char *file_name, double **values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "r"))) {
		perror("Couldn't open input file");
		return -1;
	}
	int num_values;
	if (EOF == fscanf(file, "%d", &num_values)) {
		perror("Couldn't read element count from input file");
		return -1;
	}
	if (NULL == (*values = malloc(num_values * sizeof(double)))) {
		perror("Couldn't allocate memory for input");
		return -1;
	}
	for (int i=0; i<num_values; i++) {
		if (EOF == fscanf(file, "%lf", &((*values)[i]))) {
			perror("Couldn't read elements from input file");
			return -1;
		}
	}
	if (0 != fclose(file)){
		perror("Warning: couldn't close input file");
	}
	return num_values;
}

// Dont change
int write_output(char *file_name, const double *output, int num_values) {
	FILE *file;
	if (NULL == (file = fopen(file_name, "w"))) {
		perror("Couldn't open output file");
		return -1;
	}
	for (int i = 0; i < num_values; i++) {
		if (0 > fprintf(file, "%.4f ", output[i])) {
			perror("Couldn't write to output file");
		}
	}
	if (0 > fprintf(file, "\n")) {
		perror("Couldn't write to output file");
	}
	if (0 != fclose(file)) {
		perror("Warning: couldn't close output file");
	}
	return 0;
}

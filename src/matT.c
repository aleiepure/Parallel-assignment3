/**
 * Introduction to Parallel Computing (A.A. 2023/2024)
 * Homework 3: Parallelizing matrix operations using MPI
 *
 * Parallel matrix transposition
 *
 * Alessandro Iepure
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv) {

    // Initialize MPI
    int MPI_rank, MPI_size;
    int matrix_size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_size);    

    // Handle arguments
    if (argc != 2) {
        if (MPI_rank == 0)
            fprintf(stderr, "Error - not enough arguments\n"
                            "Usage: %s <matrix size>\n", 
                    argv[0]);

        MPI_Finalize();
        return -1;
    }

    if (sscanf(argv[1], "%i", &matrix_size) != 1) {
        if (MPI_rank == 0)
            fprintf(stderr, "Error - <matrix size> not an integer\n"
                            "Usage: %s <matrix size>\n", 
                    argv[0]);

        MPI_Finalize();
        return -1;
    }

    // Check compatibility between number of processors and matrix size
    if (matrix_size % MPI_size != 0) {
        if (MPI_rank == 0)
            printf("Error - Incompatible number of MPI processes and matrix dimensions.\n");

        MPI_Finalize();
        return 1;
    }   

    // Variables
    const int LOCAL_SIZE = matrix_size / MPI_size;

    double wt1, wt2;
    
    float *matrix = (float *) malloc(sizeof(float) * matrix_size * matrix_size);
    float *transposed_matrix = (float *) malloc(sizeof(float) * matrix_size * matrix_size);
    float *local_matrix = (float *) malloc(sizeof(float) * LOCAL_SIZE * matrix_size);
    float *transposed_local_matrix = (float *) malloc(sizeof(float) * LOCAL_SIZE * matrix_size);

    // Fill matrix
    if (MPI_rank == 0) {
        for (int i = 0; i < matrix_size; i++)
            for (int j = 0; j < matrix_size; j++)
                matrix[i * matrix_size + j] = i * matrix_size + j;
    }

    // Scatter rows to the processors
    MPI_Scatter(matrix, matrix_size * LOCAL_SIZE, MPI_FLOAT, local_matrix, matrix_size * LOCAL_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Transpose local matrices
    if (MPI_rank == 0)
        wt1 = MPI_Wtime();
    for (int i = 0; i < matrix_size; i++) {
        for (int j = 0; j < LOCAL_SIZE; j++) {
            transposed_local_matrix[i * LOCAL_SIZE + j] = local_matrix[i + j * matrix_size];
        }
    }

    // Gather local matrices to process 0
    for (int i = 0; i < matrix_size; i++)
        MPI_Gather(&(transposed_local_matrix[i * LOCAL_SIZE]), LOCAL_SIZE, MPI_FLOAT, &(transposed_matrix[i * matrix_size]), LOCAL_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if (MPI_rank == 0) {
        wt2 = MPI_Wtime();
        printf("%f\n", wt2 - wt1);
    }

    // Cleanup and exit
    free(matrix);
    free(transposed_matrix);
    free(transposed_local_matrix);
    free(local_matrix);

    MPI_Finalize();
    return 0;
}

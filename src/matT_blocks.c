/**
 * Introduction to Parallel Computing (A.A. 2023/2024)
 * Homework 3: Parallelizing matrix operations using MPI
 *
 * Parallel matrix transposition, blocks
 *
 * Alessandro Iepure
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include "math.h"

int main(int argc, char **argv) {
    
    // Initialize MPI
    int MPI_rank, MPI_size;
    int matrix_size;

    MPI_Init(&argc, &argv);
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
    if (sqrt(MPI_size) * sqrt(MPI_size) != MPI_size || matrix_size % (int) sqrt(MPI_size) != 0) {
        if (MPI_rank == 0)
            fprintf(stderr, "Error - Incompatible number of MPI processes and matrix dimensions.\n");

        MPI_Finalize();
        return -1;
    }

    // Variables
    const int BLOCKS_PER_SIDE = sqrt(MPI_size);
    const int OUTER_BLOCK_SIZE = matrix_size / sqrt(MPI_size);
    const int INNER_BLOCK_SIZE = OUTER_BLOCK_SIZE / sqrt(MPI_size);

    double wt1, wt2;

    float *matrix = (float *) malloc(matrix_size * matrix_size * sizeof(float));
    float *outer_block = (float *) malloc(OUTER_BLOCK_SIZE * OUTER_BLOCK_SIZE * sizeof(float));
    float *local_block = (float *) malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(float));
    float *transposed_local_block = (float *) malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(float));
    float *transposed_outer_block = (float *) malloc(OUTER_BLOCK_SIZE * OUTER_BLOCK_SIZE * sizeof(float));
    float *transposed_matrix = (float *) malloc(matrix_size * matrix_size * sizeof(float));

    // Fill matrix
    if (MPI_rank == 0) {
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            matrix[i] = i;
        }
    }

    // Define new MPI type
    MPI_Datatype inner_block_type;
    MPI_Datatype tmp;
    MPI_Type_vector(INNER_BLOCK_SIZE, INNER_BLOCK_SIZE, OUTER_BLOCK_SIZE, MPI_FLOAT, &tmp);
    MPI_Type_create_resized(tmp, 0, sizeof(float), &inner_block_type);
    MPI_Type_commit(&inner_block_type);

    // Loop between the chunks
    if (MPI_rank == 0)
        wt1 = MPI_Wtime();
    for (int i = 0; i < sqrt(MPI_size); i++) {
        for (int j = 0; j < sqrt(MPI_size); j++) {

            // Fill outer block
            if (MPI_rank == 0) {
                for (int ii = 0; ii < OUTER_BLOCK_SIZE; ii++) {
                    for (int jj = 0; jj < OUTER_BLOCK_SIZE; jj++) {
                        outer_block[ii * OUTER_BLOCK_SIZE + jj] = matrix[i * matrix_size * OUTER_BLOCK_SIZE +
                                                                         j * OUTER_BLOCK_SIZE + ii * matrix_size + jj];
                    }
                }
            }

            // Compute values for MPI_Scatterv
            int disps[MPI_size];
            int counts[MPI_size];
            for (int ii = 0; ii < BLOCKS_PER_SIDE; ii++) {
                for (int jj = 0; jj < BLOCKS_PER_SIDE; jj++) {
                    disps[ii * BLOCKS_PER_SIDE + jj] = ii * OUTER_BLOCK_SIZE * INNER_BLOCK_SIZE + jj * INNER_BLOCK_SIZE;
                    counts[ii * BLOCKS_PER_SIDE + jj] = 1;
                }
            }

            // Scatter local blocks to the processors
            MPI_Scatterv(outer_block, counts, disps, inner_block_type, local_block, INNER_BLOCK_SIZE * INNER_BLOCK_SIZE,
                         MPI_FLOAT, 0, MPI_COMM_WORLD);

            // Transpose local blocks
            for (int ii = 0; ii < INNER_BLOCK_SIZE; ii++) {
                for (int jj = 0; jj < INNER_BLOCK_SIZE; jj++) {
                    transposed_local_block[ii * INNER_BLOCK_SIZE + jj] = local_block[jj * INNER_BLOCK_SIZE + ii];
                }
            }

            // Compute values for MPI_Gatherv
            for (int ii = 0; ii < BLOCKS_PER_SIDE; ii++) {
                for (int jj = 0; jj < BLOCKS_PER_SIDE; jj++) {
                    disps[ii * BLOCKS_PER_SIDE + jj] = ii * INNER_BLOCK_SIZE + jj * INNER_BLOCK_SIZE * OUTER_BLOCK_SIZE;
                    counts[ii * BLOCKS_PER_SIDE + jj] = 1;
                }
            }

            // Gather local blocks to process 0, the results are already transposed
            MPI_Gatherv(transposed_local_block, INNER_BLOCK_SIZE * INNER_BLOCK_SIZE, MPI_FLOAT,
                        transposed_outer_block, counts, disps, inner_block_type,
                        0, MPI_COMM_WORLD);

            // Place the whole chunk in the correct position for the final transposed matrix
            for (int ii = 0; ii < OUTER_BLOCK_SIZE; ii++)
                for (int jj = 0; jj < OUTER_BLOCK_SIZE; jj++)
                    transposed_matrix[j * matrix_size * OUTER_BLOCK_SIZE + i * OUTER_BLOCK_SIZE + ii * matrix_size +
                                      jj] = transposed_outer_block[ii * OUTER_BLOCK_SIZE + jj];
        }
    }
    if (MPI_rank == 0) {
        wt2 = MPI_Wtime();
        printf("%f\n", wt2 - wt1);
    }
    
    // Cleanup and exit
    free(matrix);
    free(outer_block);
    free(local_block);
    free(transposed_local_block);
    free(transposed_outer_block);
    free(transposed_matrix);

    MPI_Finalize();
    return 0;
}
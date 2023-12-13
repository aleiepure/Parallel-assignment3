#! /bin/bash
#
# Introduction to Parallel Computing (A.A. 2023/2024)
# Homework 3: Parallelizing matrix operations using MPI
#
# Run script
#
# Alessandro Iepure
#

# Abort on error
set -e

# Load required modules and compiler
module load gcc91
module load mpich-3.2.1--gcc-9.1.0

# Workaround cluster issues with compiler versions
gcc() {
    gcc-9.1.0 "$@"
}
gcc --version
mpicc --version

# Working directory
cd /home/$USER/assignment3

# Create directiories, if needed
mkdir -p out
mkdir -p bin

# Compile code
mpicc src/matT.c -o bin/matT.out -lm
mpicc src/matT_blocks.c -o bin/matT_blocks.out -lm

# Submit jobs to the queue
qsub job/job_np1.pbs
qsub job/job_np4.pbs
qsub job/job_np16.pbs
qsub job/job_np64.pbs

# Look at the status
watch qstat -u $USER

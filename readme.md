# Introduction to Parallel Computing (A.Y. 2023-2024) - Assignment 3

This repo contains the solution of the [third assignment](homework3.pdf) of the course "Introduction to Parallel 
Computing - prof. Vella" from the University of Trento.

## Build and run
> [!NOTE]
> The execution was validated ***ONLY*** on the HPC cluster via PBS files. Refer to those files for the commands for a "manual" execution.

Clone this repository on the University HPC cluster, and submit to the queue all the jobs with the following 
commands:

```shell
git clone https://github.com/aleiepure/Parallel-assignment3 ~/assignment3
cd ~/assignment3
sed -i "s/alessandro.iepure/$(whoami)/g" job/job_np1.pbs  # change my username to the current user's
sed -i "s/alessandro.iepure/$(whoami)/g" job/job_np4.pbs
sed -i "s/alessandro.iepure/$(whoami)/g" job/job_np16.pbs
sed -i "s/alessandro.iepure/$(whoami)/g" job/job_np64.pbs
./run.sh
```

The last command is a script that is responsible of the creation of the required folders, the compilation, and the submition
of the job files to the queue. 
The script will end by launching the command `watch qstat -u $USER` to monitor the active user's jobs. Once there are no more, press `CTRL+C` to exit.

At the end of the job, the results of the computations will be available in `stdout_np<num>.o` files in the `out/` folder 
(created automatically). \
Errors abort the execution of the job immediately and the `stderr` is dumped into `out/stderr_np<num>.e`.

## Results
The report analyzing the results is available [here](report/build/report.pdf).

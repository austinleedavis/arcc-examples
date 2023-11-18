#!/bin/bash

###########
# This is an example Slurm submission script for working with the Dask
# distributed computing framework for Python.  This script aims to be better
# than the method that Dask's docs recommend for working with Slurm, which
# can overload the scheduler.  This script is for CPU (no GPUs) jobs on the
# Stokes HPC cluster at the UCF ARCC.
#
# This example is optimized for programs that are primarily pure python code
# which does not make extensive use of libraries such as Numpy, Scipy, Numba,
# DataFrames, or anything else using Cython/C/C++/Fortran/etc... for speed.
# See https://distributed.dask.org/en/latest/efficiency.html for more details.
#
# With this script, you should *not* use any of the dask tools for launching the
# scheduler or worker processes (i.e. dask-jobqueue, dask-mpi, dask-drmaa,
# SSHCluster, etc.).  Your python code should only establish the dask client and
# then create and submit your tasks.
#
# This script uses Slurm's heterogenous job support to allow the scheduler,
# client, and worker processes to have different parameters.
# (https://slurm.schedmd.com/heterogeneous_jobs.html)
#
###########


#############################
# Specify Slurm job options #
#############################
# Remember: `#SBATCH` are slurm directives, not comments.
# Do not remove the '#' sign.

#################################################
# Job Component 1, Scheduler and Client options #
#################################################
# These options will generally apply to the whole job unless overridden in the
# later job component sections.

# Time limit, adjust as appropriate. Remember, the scheduler and client need to
# run for the entire duration of processing.
#SBATCH --time=1:00:00

# Two tasks, one for the scheduler and one for the client.
#SBATCH --ntasks=2

# Job component name
#SBATCH --job-name="dask-scheduler"

# Redirect stdout and stderr. You can change this if you wish. the '%J' token
# will be replaced with the job ID at runtime.
#SBATCH --output=dask-%J.out
#SBATCH --error=dask-%J.err

# End of first job component configuration

# Job step separator
#SBATCH hetjob

########################################
# Job Component 2, Worker options #
########################################
# These options will apply to the workers. 
# Some of the options, such as time, will be inherited
# from above.

# Job component name
#SBATCH --job-name="dask-workers-normal"

# Number of tasks, adjust as necessary but remember that the larger this is, the
# longer your job may wait in the queue and the more DPH you will consume.
#SBATCH --ntasks=20

# Amount of memory per CPU.  Since these tasks will have only 1 CPU each, this
# is also the amount of memory per task.  Adjust this as necessary. Make sure to
# match this to the same option in the third job component.
#SBATCH --mem-per-cpu=2G

# End of second job component

# Now that we have specified the options for our job components, it is time to
# actually run Dask.

# Load python/anaconda module needed
module load anaconda/anaconda3

# If necessary, activate your virtual environment/conda environment. Note:
# `conda activate <env>` probably won't work. Instead use something like:
# source activate <env>

# Location of the scheduler file, which dask uses to tell the workers and client
# how to connect to the scheduler.
export SCHED_FILE=~/dask-scheduler-${SLURM_JOB_ID}.json

# The path to the Python program containing your Dask client. See
# dask-example.py for an example of how to create your Dask client.
DASK_CLIENT=dask-example.py

# Command line arguments to be given to the Dask scheduler
DASK_SCHED_ARGS="--scheduler-file ${SCHED_FILE} --interface ib0"

# Command line arguments to be given to the Dask workers
DASK_WORKER_ARGS="--scheduler-file ${SCHED_FILE} --nthreads 1 --nprocs 1 --interface ib0"

# Start the Dask scheduler
srun --het-group=0 --ntasks=1 dask-scheduler ${DASK_SCHED_ARGS} &

# Start the base workers
srun --het-group=1 dask-worker ${DASK_WORKER_ARGS} &

# Start your dask client and begin processing
srun --het-group=0 --ntasks=1 python ${DASK_CLIENT} &

# Wait for the above processes to finish before exiting
wait

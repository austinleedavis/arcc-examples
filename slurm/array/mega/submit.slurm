#!/bin/bash

#SBATCH --ntasks=1  # One task per array task
#SBATCH --mem-per-cpu=1G # 1GB of RAM per CPU
#SBATCH --time=00:15:00 # 15 minutes per array task
#SBATCH --output=mega-array-%A_%a.out # Send STDOUT from each task to the named file
#SBATCH --error=mega-array-%A_%a.err # Send STDERR from each task to the named file
#SBATCH --job-name=mega-array-example # set job name
#SBATCH --array=1-10%5 # Array index range. <start>-<end>%<throttle>

# This example script combines Slurm array jobs with loops to process large
# numbers of *short* tasks.  Array jobs are convenient for running a lot of
# tasks, but for short tasks the scheduling overhead becomes significant, so it
# is more efficient to coalesce the tasks.  This makes your job more efficient
# while also avoiding bogging down the job queue for other users.

# The array option specified above sets our array indices to begin at 1 and end
# at 10, running at most 5 tasks at a time. As each task finishes, a new one
# starts until the range is exhausted.


# Set the number of runs for each task
PER_TASK=100

# calculate starting and ending values, based on the slurm task index.
# We can use these, for example, to iterate through numbered input files.
START=$(( (${SLURM_ARRAY_TASK_ID} - 1) * ${PER_TASK} + 1 ))
END=$(( ${SLURM_ARRAY_TASK_ID} * ${PER_TASK} ))

echo "This is task ${SLURM_ARRAY_TASK_ID} of Slurm job ${SLURM_ARRAY_JOB_ID} on node `hostname`"
echo "This task will cover runs ${START} to ${END}"

# run loop
echo "Beginning loop: `date`"

for (( run=${START}; run<=${END}; run++ )); do
	echo "TASK: ${SLURM_ARRAY_TASK_ID} RUN: ${run}"
	./run.sh inputs/${run}.txt > outputs/${run}.txt
done

echo "Ending loop: `date`"

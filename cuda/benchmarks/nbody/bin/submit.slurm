#!/bin/bash
#SBATCH --nodes=3
#SBATCH --tasks-per-node=4
#SBATCH --time=02:00:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --job-name=CUDA_Benchmarks
#SBATCH --gres=gpu:2

#MULTIPLIER IS MULTIPLIED BY 256 TO GENERATE THE NUMBER OF BODIES
MULTIPLIER=8192
BODIES=$((256*$MULTIPLIER))
#Select file to run
export file="nbody"
export args="-benchmark -fullscreen -numdevices=2 -numbodies=$BODIES"
export args2="-benchmark -fullscreen -cpu -numbodies=$BODIES"

#Select how logs get stored
mkdir $SLURM_JOB_ID
export debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
export benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"

# Load Modules
module load cuda/cuda-8.0

# Enter Working Directory
cd $SLURM_SUBMIT_DIR
# Create Log FIle
echo $SLURM_SUBMIT_DIR
echo "JobID: %SLURM_JOB_ID" >> $debug_logs
echo "Running on $SLURM_NODELIST" >> $debug_Logs
echo "Running on $SLURM_NPROCS processors." >> $debug_logs
echo "Current working directory is `pwd`" >> $debug_logs

# Module debugging information
module list >> $debug_logs

date >> $benchmark_logs
echo "ulimit -l: " >> $benchmark_logs
ulimit -l >> $benchmark_logs

#Run job
./$file $args 
#uncomment below for CPU run
#./$file $args2 > $SLURM_JOB_ID/CPU_RUN

echo "Program is finished with exit coded $? at: `date`"
sleep 3

date >> $benchmark_logs
echo "ulimit -l" >> $benchmark_logs
ulimit -l >> $benchmark_logs

mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

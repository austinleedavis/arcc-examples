#!/bin/bash
#SBATCH --nodes=3
#SBATCH --tasks-per-node=2
#SBATCH --time=00:30:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --job-name=Simple
#SBATCH --gres=gpu:2

#Select file to run
export file="Pi"
export args=""

#Select how logs get stored
mkdir $SLURM_JOB_ID
export debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
export benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"

# Load Modules
module load cuda/cuda-11.4
module load gcc

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
echo "Program is finished with exit coded $? at: `date`"
sleep 3

date >> $benchmark_logs
echo "ulimit -l" >> $benchmark_logs
ulimit -l >> $benchmark_logs

mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

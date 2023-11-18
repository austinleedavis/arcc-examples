#!/bin/bash
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=3
#SBATCH --time=00:30:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out

#Select File to run
file="PPi"
args="500000000000000"

#Select how logs get stored
mkdir $SLURM_JOB_ID
debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"

#Load Modules
module load oneapi/compiler-rt
module load oneapi/mpi

# Create Log File
echo $SLURM_SUBMIT_DIR
echo "JobID: $SLURM_JOB_ID" >> $debug_logs
echo "Running on $SLURM_NODELIST" >> $debug_logs
echo "Running on $SLURM_NNODES nodes." >> $debug_logs
echo "Running on $SLURM_NPROCS processors." >> $debug_logs
echo  "Current working directory is `pwd`" >> $debug_logs

# Module debugging
module list >> $debug_logs
which mpirun >> $debug_logs

date >> $benchmark_logs
 
# Run job
mpirun ./$file $args
echo "Program is finished with exit code $? at: `date`"
sleep 3

date >> $benchmark_logs
mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

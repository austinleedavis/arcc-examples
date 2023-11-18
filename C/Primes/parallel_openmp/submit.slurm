#!/bin/bash
#SBATCH --nodes=5
#SBATCH --ntasks-per-node=5
#SBATCH --time=00:30:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out

#Select File to run
file="prime"
arguments=""

#Select how logs get stored
mkdir $SLURM_JOB_ID
debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"

#Load Modules
module load openmpi/openmpi-2.0.1-gcc-6.2.0

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
mpirun ./$file $arguments 
echo "Program is finished with exit code $? at: `date`"
sleep 3

date >> $benchmark_logs
mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

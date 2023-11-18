#!/bin/bash
###################### Total Tasks #######################
## This is an script template designed for ease of use  ##
## With a job distribution based on the number of tasks ##
## a person would like and how they would like those    ##
## tasks to be distributed among the nodes              ##
##             Total_Procs == n                         ##
##########################################################

#SBATCH -A arcc
#SBATCH -n 6
#SBATCH --ntasks-per-node=3
#SBATCH --time=00:30:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --job-name=SimpleJob


#Select File to run
export file=""
export args=""

#Select how logs get stored
mkdir $SLURM_JOB_ID
export debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
export benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"



#Load Modules

# Enter Working Directory
cd $SLURM_SUBMIT_DIR
# Create Log File
echo $SLURM_SUBMIT_DIR
echo "JobID: $SLURM_JOB_ID" >> $debug_logs
echo "Running on $SLURM_JOB_NODELIST" >> $debug_logs
echo "Running on $SLURM_JOB_NNODES nodes." >> $debug_logs
echo "Running on $SLURM_JOB_NPROCS processors." >> $debug_logs
echo  "Current working directory is `pwd`" >> $debug_logs

# Module debugging
module list >> $debug_logs
which mpirun >> $debug_logs

#Start Timestamp
date >> $benchmark_logs
echo "ulimit -l: " >> $benchmark_logs
ulimit -l >> $benchmark_logs
 
# Run job
srun ./$file $args
echo "Program is finished with exit code $? at: `date`"
sleep 3

#End Timestamp
date >> $benchmark_logs
echo "ulimit -l" >> $benchmark_logs
ulimit -l >> $benchmark_logs

#Cleanup
mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

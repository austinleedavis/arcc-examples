#!/bin/bash
#################### Total Nodes #########################
## This is an script template designed for ease of use  ##
## With a job distribution based on the number of nodes ##
## a person would like and how many tasks they would    ##
## like on each node.                                   ##
##      Total-Procs == nodes*ntasks-per-node            ##
##########################################################


#SBATCH --nodes 1
#SBATCH --ntasks-per-node=6
#SBATCH --time=00:30:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --job-name=helloworld-parallel-matlab


#Select File to run
export file="test"
export args=""

#Select how logs get stored
mkdir $SLURM_JOB_ID
export debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
export benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"

#Load Modules
module load matlab/matlab-R2018b
module load openmpi/openmpi-3.0.1-gcc-6.2.0

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
./$file $args
echo "Program is finished with exit code $? at: `date`"
sleep 3

#End Timestamp
date >> $benchmark_logs
echo "ulimit -l" >> $benchmark_logs
ulimit -l >> $benchmark_logs

#Cleanup
mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

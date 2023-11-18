#!/bin/bash
#SBATCH --ntasks=4
#SBATCH --nodes=1 ##GAMESS doesn't seem to want to work with multinode setups...
#SBATCH --time=02:00:00
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --constraint="westmere"

#Select File to run
export file=""
export arguments=""

#Select how logs get stored
mkdir $SLURM_JOB_ID
export debug_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"
export benchmark_logs="$SLURM_JOB_ID/job_$SLURM_JOB_ID.log"


#Load Modules
module load gamess/gamess-19.06-openmpi-4.0.0-gcc-9.1.0
#module load gamess/gamess-17.04

# Enter Working Directory
cd $SLURM_SUBMIT_DIR
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
echo "ulimit -l: " >> $benchmark_logs
ulimit -l >> $benchmark_logs
 
# Run job
if [ "${SLURM_ARRAY_TASK_ID}" -lt "10" ]; then
	rungms exam0${SLURM_ARRAY_TASK_ID}.inp > exam0${SLURM_ARRAY_TASK_ID}.log
	echo "SLURM TASK ID IS " + $SLURM_ARRAY_TASK_ID
else
	rungms exam${SLURM_ARRAY_TASK_ID}.inp > exam${SLURM_ARRAY_TASK_ID}.log
fi
#mpirun ./$file $arguments 
echo "Program is finished with exit code $? at: `date`"
sleep 3

date >> $benchmark_logs
echo "ulimit -l" >> $benchmark_logs
ulimit -l >> $benchmark_logs
mv job.$SLURM_JOB_ID.err $SLURM_JOB_ID/
mv job.$SLURM_JOB_ID.out $SLURM_JOB_ID/

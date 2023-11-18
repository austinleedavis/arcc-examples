#!/bin/bash

#SBATCH --job-name=simple_parallel
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --time=00:30:00
#SBATCH --ntasks-per-node=30
#SBATCH --nodes=1
#SBATCH -L mathematica@mathematica.net.ucf.edu

mathematica_script=prime-number-20k.m

module load mathematica
module load openmpi/openmpi-2.0.1-gcc-6.2.0
mkdir $SLURM_JOB_ID

mpirun math -run < ${mathematica_script} > job.${SLURM_JOB_ID}.out

mv job.${SLURM_JOB_ID}.err $SLURM_JOB_ID/
mv job.${SLURM_JOB_ID}.out $SLURM_JOB_ID/





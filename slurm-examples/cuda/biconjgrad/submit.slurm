#!/bin/bash
#SBATCH --gpus=1
#SBATCH --cpus-per-gpu=2
#SBATCH --gres-flags=enforce-binding
#SBATCH --time=00:10:00
#SBATCH --job-name=GPU-Example

# Give this process 1 task (per GPU, but only one GPU), then assign two cores per task
# (so two cores overall).  Then enforce that slurm assigns only CPUs that are on the
# socket closest to the GPU you get.

# If you want two GPUs:
# #SBATCH --gpus=2

#Select between the different Solvers and Problems by changing the SOLVER and PROBLEM Variables
SOLVER=biconjgrad
#SOLVER=conjgrad

PROBLEM=ThermalBarrierCoating
#PROBLEM=TurbineBladeStresses


#These env vars will help
NUM_GPUS=`echo $GPU_DEVICE_ORDINAL | tr ',' '\n' | wc -l`

# Output some preliminaries before we begin
date
echo "Slurm nodes: $SLURM_JOB_NODELIST"
NUM_GPUS=`echo $GPU_DEVICE_ORDINAL | tr ',' '\n' | wc -l`
echo "You were assigned $NUM_GPUS gpu(s)"

# Load modules
module load gcc/gcc-8.3.0
module load cuda/cuda-10.2

# List the modules that are loaded
module list

# Have Nvidia tell us the GPU/CPU mapping so we know
nvidia-smi topo -m

# Okay, finally run the darned code
echo
echo Run the solver...
echo 
./bin/${SOLVER} data/${PROBLEM}/K.ell data/${PROBLEM}/f.vct  data/${PROBLEM}/my-solution.vct  0 $NUM_GPUS
echo

# You're done!
echo "Ending script..."
date

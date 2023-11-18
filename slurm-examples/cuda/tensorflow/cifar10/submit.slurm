#!/bin/bash

#SBATCH --time=00:15:00
#SBATCH --cpus-per-gpu=4
#SBATCH --gpus=1
#SBATCH --output=cifar10-train-slurm-%J.out

# Give this process 1 task (per GPU, but only one GPU), then assign eight 8per task
# (so 8 cores overall).  Then enforce that slurm assigns only CPUs that are on the
# socket closest to the GPU you get.

# If you want two GPUs:
# #SBATCH --gpus=2


# Output some preliminaries before we begin
date
echo "Slurm nodes: $SLURM_JOB_NODELIST"
NUM_GPUS=`echo $GPU_DEVICE_ORDINAL | tr ',' '\n' | wc -l`
echo "You were assigned $NUM_GPUS gpu(s)"

# Load the TensorFlow module
module load anaconda/anaconda3

# List the modules that are loaded
module list

# Have Nvidia tell us the GPU/CPU mapping so we know
nvidia-smi topo -m

echo

# Activate the GPU version of TensorFlow
source activate tensorflow-gpu

# OR, instead:  Activate the non-GPU version of TensorFlow
#source activate tensorflow


# Run TensorFlow
echo
time python models/tutorials/image/cifar10/cifar10_train.py  --train_dir=./out  --tmp_dir=./tmp
echo

# You're done!
echo "Ending script..."
date



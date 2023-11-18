#!/bin/bash

#SBATCH --nodes=8
#SBATCH --ntasks-per-node=8
#SBATCH --time=01:00:00
#SBATCH --mem=30G

# Setup the simulation
module load openfoam/openfoam-6.0-openmpi-3.1.0-gcc-7.1.0
source $foamDotFile
export WM_LABEL_SIZE=64

# Run the simulation
cd sloshingTank2D
./Allrun

#!/bin/bash

#SBATCH --time=1:00:00
#SBATCH --ntasks=1

module load turbovnc
module load virtualgl

vncserver :1 -fg

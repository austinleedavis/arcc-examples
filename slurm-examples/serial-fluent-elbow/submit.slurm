#!/bin/bash
#
#SBATCH -n 1
#SBATCH -t 1:00:00
#SBATCH -J simple-fluent

module load ansys-fluids/ansys-fluids-16.0

export FLUENT_GUI=off

time fluent 3d -g -i simple.jrnl

#!/bin/bash
##SBATCH --job-name=g09-test

# max run time
#SBATCH --time=10:00:00


# 10 servers, 8 linda worker per server
#SBATCH --nodes=10
#SBATCH --ntasks-per-node=8

# Specify slurm partition

# activate bash debugging from here
set -x

# Name of your gjf file ie l2-PtCl-td.gjf
OB=g09-test
OUTPUT=OUTPUT.TXT
export JOB=g09-test.com

# This creates a list of nodes that you job received to run on
LindaList=./nodes_linda.$SLURM_JOBID
touch $LindaList

# This creates a jobfile


module load gaussian/gaussian-g09a2linda
#module load gaussian/gaussian-g16b01

# Create a list of hostnames and save it to the LindaList machine file
scontrol show hostname ${SLURM_NODELIST} | sort -u > $LindaList

# Tell linda to use ssh
export GAUSS_LFLAGS=' -opt "Tsnet.Node.lindarsharg: ssh"'

# Read the contents of the machine file and put it in the job file
workers="%LindaWorkers="$(cat $LindaList | tr "\n" "," | sed "s/,$//")

# Run gaussian using our job file and output to a matching results file
g09 < $JOB  



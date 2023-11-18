#!/bin/bash
#SBATCH --job-name=JN-%u
#SBATCH --output=JN-%j.log
#SBATCH --ntasks=1
#SBATCH --mem=8gb
#SBATCH --time=02:00:00

module load anaconda/anaconda3

# If the port is not passed in, then get the user's UID
if [[ $# -lt 1 ]] ; then
  PORT="$UID"
fi

# If the User Token is not set, set it to 'Test'
if [[ ! $USER_TOKEN ]] ; then
  USER_TOKEN="Test"
fi


# Helpful for Debugging
echo "-------------------------------"
echo "Job Information:"
date
hostname
pwd
module list
echo "SLURM Nodes:  ${SLURM_NODELIST}"
echo "JN Port: $PORT"
echo "-------------------------------"


mkdir $HOME/tmp >> /dev/null  2>&1
export XDG_RUNTIME_DIR=$HOME/tmp


jupyter notebook --no-browser --port=$1 --ip='*' --NotebookApp.token=$USER_TOKEN

date


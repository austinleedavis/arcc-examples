#######################################################################
#
# This is an example to show how you can use a singularity container
# to run TensorFlow 2.0 jobs in interactive mode on Newton.  The job
# is a simple convolutional neural network solution to the MNIST
# character recognition problem.
#
# Before you do anything else, please **copy** all the contents
# to your local area.  **DO NOT** try to run it from the examples
# directory on Stokes/Newton. 
#
# You can run this on Newton or Stokes, but TF will not use the
# gpu unless it has access to one. This means it will run slower
# on Stokes.
#
#  mkdir <wherever-you-want-this>
#  cd <wherever-you-want-this>
#  cp -R /examples/current/singularity-TF2 .
#
#######################################################################

Load the singularity module:
 
  module load singularity


Pull the container if you've not done so yet.  You only need to pull a
container once.  If the sif file exists, you don't have to do this again.

  singularity pull docker://tensorflow/tensorflow:latest-gpu

Don't worry about the "gpu" part for Stokes.  The container is configured
with a version of TensorFlow that is smart enough to fail over to the CPU
if it cannot access CUDA libraries or a GPU device.


Request an interactive job on a node.  Omit the "--gpus=1" if running
on Stokes:

  srun -n4 --gpus=1  --time=00:30:00  --pty bash



Now you should be on a node with the requested resources.  You'll have
30 minutes to work (the --time parameter, above).

To run the container, use the following:  

  singularity run --nv tensorflow_latest-gpu.sif


To run the example, type:
  python3 SimpleConv2DExample.py


#######################################################################
#
# This directory is represents an example hand-coded CUDA application
# to serve as an example for users on building and running such 
# applications on Newton.
#
# Before anything else, please **copy** all contents to your local
# area.  **DO NOT** try to run here in the example area.
#
#  mkdir <wherever-you-want-this>
#  cd <wherever-you-want-this>
#  cp -R /examples/current/cuda/biconjgrad .
#
#######################################################################
 


Author:  R. Paul Wiegand


Project: NASA/JPL and Intelligent Automations Inc.
         Examining different GPU and non-GPU based methods
         to solve complicated, very large, sparse-matrix
         problems.


Programs: There are solvers in this project.

  biconjgrad	This is a GPU-based bi-conjugate gradient
		solver that is numerically stable.  It reads
		a special ELL formatted data matrix and vectors.

  conjgrad	This is a GPU-based conjugate gradient solver
		that is NOT numerically stable.  It also reads
		a special ELL formatted file.


Data:	In the ./data directory are two example problems.
	These include a ballistics reentry problem and turbine
	blade stress problem.  Read the readme.txt in each
	of these to learn more about the data.


Directories:
   ./src	C++ and Cuda source code
   ./inc	C++ and Cuda header code
   ./obj	Temporary directory to stash object files during build
   ./bin	Directory where binaries are built
   ./data       Directory containing input and output data for each of the examples.


How to build:	Make sure a CUDA and GCC compiler are in path.
		It has been most recently tested using Cuda 10.1
		and GCC 8.3.0.  Note that Cuda 9.0 is incompatible
		with later versions of GCC.

		On Newton from the head node:

  # Request an interactive session
  srun -n2 --gres=gpu:1 --time=10:00  --pty bash

  # Change to the directory where the source is
  cd <whereever-you-put-this-code>

  # Load the requisite modules
  module load gcc/gcc-8.3.0
  module load cuda/cuda-10.2

  # Make the project
  make clean
  make

  # Exit back to the head node
  exit


How to run the code: 	Make sure Cuda and GCC libs are in
			LD_LIBRARY_PATH (e.g., load those modules).
			Then run the binary without arguments:

   <wherever-you-put-this-code>/bin/biconjgrad			

			This will tell you how the command-line works.

			For example:

  # Request an interactive session
  srun -n2 --gres=gpu:1 --time=1:00:00  --pty bash

  # Change to the directory where the sourse is
  cd <whereever-you-put-this-code>

  # Load the requisite modules
  module load gcc/gcc-8.3.0
  module load cuda/cuda-10.1

  # Run the code:
  bin/biconjgrad data/ThermalBarrierCoating/K.ell data/ThermalBarrierCoating/f.vct  data/ThermalBarrierCoating/my-solution.vct  0 1


			Or just subject the job from the head node:

  cd <wherever you put this code>
  sbatch submit.slurm



What the output should look like:	If the code runs correctly, you will
					see a slurm-<jobid>.out file.  The contents
					of that will show some preliminary information
					regarding what resources were requested and how
					SLURM assigned GPUs to CPUs, then the output
					of the program.  

					The output will include information about the
					size and density of matrices, as well as timing
					information for assembly and solving and the 
					normed error.  

 					The ./data/<Problem>/solution.vct file will have
					the true solution.  The solver will have produced
					a file called my-solution.vct, to which you may
					compare the results.



#######################################################################
#
# This directory represents an example for a 2D laminar flow simulation
# using OpenFOAM 6.0.  It simulates a 2D tank sloshing example.
#
# Before you do anything else, please **copy** all the contents
# to your local area.  **DO NOT** try to run it from the examples
# directory on Stokes.  Also, **DO NOT** run this on Newton.  It 
# does not use GPUs, and will waste resources other people need.
#
#  mkdir <wherever-you-want-this>
#  cd <wherever-you-want-this>
#  cp -R /examples/current/openfoam6-sloshing2D .
#
#######################################################################
 

Directory contents:
   readme.txt           This help file
   submit.slurm         The example submissions script 
   ./sloshingTank2D	The OpenFOAM tutorial example directory

How to run the code:

  cd <wherever you put this code>
  sbatch submit.slurm


NOTE:
Once you run the simulation, a lot of material will be generated
inside the sloshingTank2D directory.  Running a second time will
be problematic.  The simplest way to clean it is to run the following
script from your local version of the example directory.

If interested, this is just one of many examples that come with
OpenFOAM.  In this case, it is the following example: 
/apps/openfoam/openfoam-6.0-openmpi-3.1.0-gcc-7.1.0/OpenFOAM-6/tutorials/multiphase/interFoam/laminar/sloshingTank2D 

The simulation should take about 2 minutes.

After that, you can copy the entire sloshingTank2D directory to your local
machine and use Paraview to visualize it.  

https://www.paraview.org/download/

1) Download the entire sloshingTank2D directory from your local area on 
   Stokes to your local computer.

2) Run Paraview (5.x or higher).

3) Load the simulation:
   + Select File->Open
   + Navigate to the sloshingTank2D/system Directory
   + Change the file type to "All Files *.*"
   + Select the file controlDict and open it
   + When prompted, find the "OpenFOAM Reader", press "OK"

4) Prepare for visualization:
   + Press the little greyed out eye icon next to controlDict in
     the Pipeline Browser (left-side of the window)
   + Press the +X orientation button at the top/center menu

5) Hit the run button at the top (green triangle).


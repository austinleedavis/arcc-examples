--------------------------------------------------------------------------------------
Author:        R. Paul Wiegand
Date Created:  2018-11-26
Modified By:  List <name, date> below
--------------------------------------------------------------------------------------

This example contains three files:  the two scripts for starting and stopping 
Jupyter Notebooks (JN), and a subordinate slurm script for requesting resources.
Please *copy* these scripts into some local directory in your user space, then
use them there.  Do *not* run them from the examples directory. For example,
you could run the following:

  cd ..   #go to this directory's parent directory
  cp -r jupyter-notebooks ~   #recursively copy the directory to your home directory
  cd ~/jupyter-notebooks   #go to the newly copied directory

You only need to do the previous step *once*.

To run JN, log into the user node on whichever cluster, then run the following
script:

  ./startJupyterNotebooks.sh

The script will submit a slurm batch job for you, spin up JN, then give you
a means of connecting via a web browser.

When you are done, please remember to stop JN using the script:

  ./stopJupyterNotebooks.sh


By default, the start script will request 1 core on 1 node with 8GB of memory
for two hours.  If you want a different resource request, please modify the
jupyter-notebooks.slurm file.


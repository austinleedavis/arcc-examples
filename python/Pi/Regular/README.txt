--------------------------------------------------------------------------------------
Author:        R. Paul Wiegand
Date Created:  2018-11-26
Modified By:  List <name, date> below
--------------------------------------------------------------------------------------

This example contains three files:  the submit script and the python script to execute.
Please *copy* these scripts into some local directory in your user space, then
use them there.  Do *not* run them from the examples directory. For example,
you could run the following:

  mkdir ~/python-example
  cd ~/python-example
  cp /groups/arcc/shared/examples/current/python/Pi/Regular/*  .

You only need to do the previous step *once*.

To submit the job, type:
  sbatch submit-py.slurm

The system will give you a JOBID.  You can see the queue status of your job 
with:

  squeue -j <JOBID>

Don't just copy and paste that last command.  Change "<JOBID>" to the actual
JOBID your received from sbatch.

When the job is done, it will disappear from the queue.  You will find the
results in a directory with the same name as your JOBID:

   ./job<JOBID>.err
   ./job<JOBID>.out



# Purpose
This repository is a copy of the existing ARCC workflow management examples. 
These examples are available on the system (e.g., Newton) in the following directory: `/groups/arcc/shared/examples/Slurm`
However, exploring these files remotely through terminal can be tedious. 
Therefore, I've uploaded these examples to GitHub to facilitate exploration, copying, development, etc.

## Preparation
This repository is meant as a reference for getting started with slurm and the ARCC clusters, but the examples included entire working projects with their datasets and outputs.
I trimmed these projects down in several ways:
- removed nested .git repositiories
- removed all files larger that 1M
- removed *most* image files (jpg, png, etc)
- removed directories with 'dataset' in the name

Some unnecessary remnants still exist (e.g., output.txt files), but my goal was to simply reduce the overall size and file count, rather than reach a 100% clean state.


# ARCC Workflow management

Workflow on an HPC typically involves submitting jobs to a batch workload manager, which then wait until sufficient compute resources are available to execute the job.  Scheduling, resource management, and accounting of usage on ARCC resources such as Stokes and Newton are handled by the Simple Linux Utility for Research Management ([SLURM](http://slurm.schedmd.com/ "Slurm Website")).  In order to use our computing facilities, you will have to learn some of the basics of job and account management in SLURM.

### Job Management

To create a job, typically you need to write a _submit script_.  A submit script is just like any other unix shell scrip except that it also contains directives to the workload manager that tell it how many resources you need and for how long, etc.  Submit scripts are submitted to SLURM, which assigns it a _job id_ and puts the job in a _partition_ (SLURM calls queues "partitions").  You can see what partitions are available and for what nodes by typing the following command at the unix command-line:

```sh
$ sinfo  
PARTITION   AVAIL  TIMELIMIT  NODES  STATE NODELIST  
normal\*        up   infinite    178  Idle\* ec\[1-8,155-324\]  
preemptable    up 2-00:00:00    178  Idle\* ec\[1-8,1550324\]
```

You can see the status of all running and pending jobs using the following command:

```sh
$ squeue  
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)  
             37    normal    SocNetPD pwiegand  R      15:32      2 ec\[7-8\]
```

To create a SLURM submit script, use an editor to create a text file.  Preface all SLURM directives on separate lines with _#SBATCH_.

```sh
$ vi simple-submit.slurm
```

Here is a very simple example MPI script:

```slurm
#!/bin/bash  
#SBATCH --account=pwiegand  
#SBATCH --nodes=4  
#SBATCH --ntasks-per-node=10  
#SBATCH --time=01:00:00  
#SBATCH --error=myjobresults-%J.err  
#SBATCH --output=myjobresults-%J.out  
#SBATCH --job-name=SimpleMPIJob  

  
\# Load modules  
echo "Slurm nodes assigned :$SLURM\_JOB\_NODELIST"  
module load openmpi-1.8.6-ic-2015.3.187  
mpirun ./hello
```

The meaning of each of these parameters are as follows:

*   **account**  indicates which account you are using (see below);
*   **nodes** indicates how many compute nodes you are requesting;
*   **ntasks-per-node** indicates how many cores per compute node you would like;
*   **time** indicates how long you expect the job to take (in terms of walltime, not cumulative compute time);
*   **error** specifies the name of the file into which the error stream of your job will be directed (the %J argument gives the job ID);
*   **output** specifies the name of the file into which the output stream of your job will be directed (the %J argument gives the job ID);
*   **job-name** specifies the name of the job as you will see it in _squeue_, for example.

You should try to be as accurate as possible with the _time_ parameter.  If it is short, your job will be killed before it finishes.  If it is too long, it may take the scheduler longer to find resources than it should.  By default, your job is given just under 2 GB of memory per core (specifically 1990 MB).  If your job tries to use more than this, it will most likely crash.  If you need more, you must request more using the _\--mem-per-cpu=<amount-in-MB>_ parameter.  Our SLURM configuration will not let a job use more memory or cores than were requested.

To submit this script to the scheduler, type:

```sh
$ sbatch simple-submit.slurm
```

The job will be placed in a queue and then deployed to the resources assigned to it as soon as the workload manager can find those resources.  When the job executes, it will begin _in the same directory as you were when you submitted_.  So if you submit from the directory in which you would like to run, you do not need to change to that direct in your submit script.  Our builds of MPI are smart enough to know which nodes you were assigned and to use the Infiniband network, so you do not have to create a hosts file; however, if you do need to know what resources were assigned to you from inside the submit script, you can use the _$SLURM\_JOB\_NODELIST_ environmental variable.
  
SLURM provides several ways to interact with the workload manager.  For example, if you just want to run 26 copies of particular project wherever you can find them, type:

```
$ srun --ntasks=26 ./myexecutable
```

Or, if you just want SLURM to allocate resources for you and then figure out what to do with them once you get them, you can use _salloc_.  The _salloc_ command will create a new shell on the login node and allocate the requested resources (once they are available).  If you want to use them, you will have to either use _srun_ or ssh to one of the nodes you were assigned.  You can use this idea to get an _interactive session_.  Indeed, we strongly advise you do so for large compilations since the ulimits for users on the login nodes are more limited than on the compute nodes. 

The following is an example shell session using _salloc_.

```sh
$ salloc --nodes=2 --ntasks-per-node=3  --time=00:30:00  
salloc: Granted job allocation 100  
$ srun hostname  
ec155  
ec155  
ec155  
ec156  
ec156  
ec156  
$ ssh ec155  
Last login: Sun Dec 20 09:32:03 2015 from euser1  
$ echo "I get three whole cores here!"  
I get three whole cores here!  
$ exit  
logout  
Connection to ec155 closed.  
$ exit  
exit  
salloc: Relinquishing job allocation 100
```

If you need to cancel the job before it has completed, type:

```sh
$ scancel _<job-id-number>  
```
_

To learn more about your job, either while it is waiting to run (_pending_) or while it is running, type:

```sh
$ scontrol show job _<job-id-number>_
```

If your job is waiting in the queue and you are confused about why it is not running while other jobs seem to be, you can use that command to learn more.  Also, _squeue_ provides an abbreviated "_REASON_" column to give you some idea whether there is a problem with your job.  Keep in mind that prioritization of jobs is a complicated function of several factors, including recent usage.  Those who use system resources less have higher priority than those who use it more -- this is known as _FairShare_.  Older jobs (those that have been in the queue longer) also have increased priority.  You can see the relative priorities of all waiting jobs with the following command:

```
$ sprio
```

Currently, we have only two partitions (queues):  _normal_ and _preemptable_.  By default, jobs are queued into the _normal_ partition.  There are no constraints on this partition, but once the account being charged for the hours has reached its limit for the month (see the next subsection) no more jobs will be executed in the _normal_ partition and running jobs will be killed.  If you there are compute resources available and you still have work to do, you can submit your job to the _preemtable_ partition.  Jobs submitted to this partition have a lower priority than jobs submitted to the _normal_ partition and, as the name implies, jobs running under that partition can be preempted by jobs other jobs.  That is, if there is a job with a higher priority requesting resources that are otherwise unavailable, SLURM will _cancel_ the preemptable job in order to make space for the higher priority job.  To submit to the _preemptable_ partition, type:

```sg
$ sbatch --partition=preemptable  --qos=preemptable  simple-submit.slurm
```

It is generally best if distributed jobs run on nodes attached to the same Infiniband switch.  Luckily, you don't have to specify switches to try to keep your jobs on one switch.  SLURM knows the topology of our network and _tries_ to find resources for a job that are all on one switch, _if that is possible_. 

For more comprehensive information about using SLURM and job directives, consult their [_Quick Start User's Guide_](http://slurm.schedmd.com/quickstart.html "SLURM Quick Start User's Guide").  You can also scan over these [slides from a HUGO event](http://eecs.ucf.edu/~wiegand/presentations/Wiegand2015hugo4.pdf "HUGO: Introduction to SLURM") last year.  Also, NERSC maintains a useful [Slurm-To-Torque translative page](https://www.nersc.gov/users/computational-systems/cori/running-jobs/for-edison-users/torque-moab-vs-slurm-comparisons/ "Slurm vs. Torque").

**Interactive Session**

Sometimes when debugging, compiling, or running certain softwares, it will be neccesary to run your job interactively. In order to submit an interactive session, you can use the following command:

```sh
$ srun --pty bash 
```

The above command will automatically use the default parameters that is set up with stokes, and log you directly into the node that is associated with your job. You can also specify special parameters just as you would in any submission script script, if anything that differs from the defaults needs to be specified. They follow the same syntax as those in the "#SBATCH" lines in your script.

An example interactive session command submitted with the same parameters as the example MPI script above:

```sh
$ srun --account=\`id -g -n\` --nodes=4 --ntasks-per-node=10 --time=01:00:00 --job-name=SimpleMPIJob --pty bash 
```

### Account Management

Every user on ARCC resources is associated with a particular SLURM _account_.  Typically, a principle investigator (PI) is responsible for an account and the "users" of that account are he or she and his or her students.  There is a limit to the compute resources available to each account each month.  On Stokes, currently each account is given at least 80,000 dedicated processor hours to spend.  SLURM prefers to report this number in _minutes_, so the standard monthly allocation is 4,800,000 minutes. 

Your usage is a total of all the processor time you have consumed.  For example, if you run a job for 10 minutes on 2 nodes using 6 cores on each node, you will have consumed two hours of compute time (10\*2\*6=120 minutes).  You can see your usage from the beginning of the month by typing the following command, replacing _pwiegand_ with your username and _12/1/15_ with the start date in which you are interested:

```sh
$ sreport cluster AccountUtilizationByUser start=12/1/15  
  
\--------------------------------------------------------------------------------  
Cluster/Account/User Utilization 2015-12-01T00:00:00 - 2015-12-17T23:59:59 (1468800 secs)  
Use reported in TRES Minutes  
\--------------------------------------------------------------------------------  
  Cluster         Account     Login     Proper Name       Used     Energy   
\--------- --------------- --------- --------------- ---------- ----------   
   stokes            root                                  195          0   
   stokes            root      root            root          4          0   
   stokes        pwiegand                                  191          0   
   stokes        pwiegand    nlucas                          3          0   
   stokes        pwiegand  pwiegand                        188          0 
```
You can also use _sshare_ to see your usage in terms of how it relates to your job priority:

```sh
$ sshare  
             Account       User  RawShares  NormShares    RawUsage  EffectvUsage  FairShare   
\-------------------- ---------- ---------- ----------- ----------- ------------- ----------   
pwiegand               pwiegand          1    1.000000       11280      1.000000   0.333333 
```

The _RawUsage_ column here reports usage in _seconds_.  _EffectiveUsage_ is a ratio that gives a sense for how much of the cluster resources that have been used were used by you relative to others in your account.  The _FairShare_ column gives your user's current fair share value.  This is a number between 0 and 1.  The more resources you consume relative to everyone else on the cluster, the lower that number gets and the lower your job priority is.  This ensures no one monopolizes the resources unfairly. You can find out more information about [FairShare here](http://slurm.schedmd.com/priority_multifactor.html "FairShare in SLURM").

To determine what the constraints are for your account, you can use the following command and look for the _cpu=<minutes>_ in the _GrpTRESMins_ column.

 ```sh
$ sacctmgr show qos arcc

     Name   Priority  GraceTime    Preempt PreemptMode  Flags UsageThres UsageFactor       GrpTRES   GrpTRESMins   
\---------- ---------- ---------- ---------- ----------- ------ ---------- ----------- ------------- -------------  
      arcc        100   00:00:00                cluster                      1.000000                 cpu=4800000                                   
```

You can also see recent history of your account using sacct:

```sh
$ sacct -X

       JobID    JobName  Partition    Account  AllocCPUS      State ExitCode   
\------------ ---------- ---------- ---------- ---------- ---------- --------   
85                 bash     normal   pwiegand         10 CANCELLED+      0:0   
86                 bash     normal   pwiegand          6    TIMEOUT      1:0   
87                 bash     normal   pwiegand          1 CANCELLED+      0:0   
88                 bash     normal   pwiegand          1 CANCELLED+      0:0   
89                 bash     normal   pwiegand          1 CANCELLED+      0:0   
90           small-tes+ preemptab+   pwiegand         12  PREEMPTED      0:0
```

You can find more information about [SLURM accounting here](http://slurm.schedmd.com/accounting.html "SLURM Accounting").

### Newton-Specific Information

Newton is a smaller cluster with some specialized GPU resources.  Consequently, the monthly allocation is lower on Newton (10,000 dedicated processor hours of CPU time and 2,000 hours of GPU time).  Also, since users will typically be requesting specialized resources, you need to know how to specify this in your submit script.  The parameter of interest here is _\--gres_ (for generic resource).   To indicate that you would like such resources, you specify the resource and the number, for example:  _\--gres=gpu:2_.  Also, because Newton's resources are optimized so SLURM is smart enough to schedule the cores "closest" to the GPU, you will typically want to ensure you are using one CPU core per task. Here is an example submit script that asks for one node, four cores per node, and two GPUs on each node:

```slurm
#!/bin/bash  
#SBATCH --account=pwiegand  
#SBATCH --nodes=1  
#SBATCH --ntasks-per-node=4  
#SBATCH --cpus-per-task=1   
#SBATCH --time=01:00:00  
#SBATCH --error=myjobresults-%J.err  
#SBATCH --output=myjobresults-%J.out  
#SBATCH --job-name=SimplePhiJob  
#SBATCH --gres=gpu:2  
  
\# Load modules  
echo "Slurm nodes assigned :$SLURM\_JOB\_NODELIST"  
module load cuda/cuda-9.0  
./mygpuprogram
```

You can find more information about [SLURM Generic Resources](http://slurm.schedmd.com/gres.html "SLURM Generic Resources").

#!/usr/bin/env python

##########
# This is an example program using the Dask distributed processing framework.
# It is intended to be launched by one of the Slurm batch scripts included in
# this directory.
#
# This example is based on the Dask Bag example:
# https://examples.dask.org/bag.html
##########

import dask
import dask.bag as db
from dask.distributed import Client
import json
import os

# First, let's get some info from our environment.
schedulerFile = os.fspath(os.getenv("SCHED_FILE"))
slurmJobID = os.getenv("SLURM_JOB_ID")

# Make a place to put our outputs
outputDir = 'output-' + slurmJobID
os.mkdir(outputDir)

# Now let's create a dask client and connect it to the scheduler
client = Client(scheduler_file=schedulerFile)

# Import some JSON data into a Dask Bag, which was generated ahead of time.
myBag = db.read_text('data/*.json').map(json.loads)

# Build pipeline(s) to run on the data

# this counts the frequencies of occupations held by people over the age of 30
# in our example data, and returns the top 100 most frequent ones.
task1 = (myBag.filter(lambda record: record['age'] > 30)
           .map(lambda record: record['occupation'])
           .frequencies(sort=True)
           .topk(100, key=1))


# Let's run the computations!
result1 = task1.compute()
# Let's stuff this in an output file!
outFile = open(outputDir + '/jobs-over-30.json', mode='x')
json.dump(result1, outFile)
outFile.close()

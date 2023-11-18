#!/bin/bash

inVal=`cat $1` #take our input from the file named in the first argument to this script

#print the value from the input multiplied by two
printf "%d * 2 = %d\n" ${inVal} $((inVal * 2))

#!/bin/bash

CLUSTER=`sacctmgr show cluster  -P | tail -n 1 | cut -f1 -d "|"`
JOBID=`cut -f1 -d";" ~/.jupyter-${CLUSTER}-shutdown-information`
SSH_TUNNEL_PID=`cut -f2 -d";" ~/.jupyter-${CLUSTER}-shutdown-information`
NODE_LIST=`cut -f3 -d";" ~/.jupyter-${CLUSTER}-shutdown-information`


echo -e "Canceling job $JOBID running on nodes ${NODE_LIST}..."
scancel $JOBID

echo -e "Killing SSH tunnel at PID $SSH_TUNNEL_PID"
kill -9 $SSH_TUNNEL_PID



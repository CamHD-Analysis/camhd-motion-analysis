#!/bin/bash

source "/home/amarburg/.rvm/scripts/rvm"


cd /home/amarburg/CamHD_motion_metadata && git pull --ff-only 


export INJECT_WINDOW=5 
cd /home/amarburg/camhd-motion-analysis-deploy && rake gcloud:inject_recent


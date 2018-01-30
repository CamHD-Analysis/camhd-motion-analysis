#!/bin/bash

echo "launch_worker.sh"

if [ ! -f /output/CamHD_motion_metadata/README.md ]; then
  echo "Could not find output directory /output/CamHD_motion_metadata"
  exit
fi

/code/camhd_motion_analysis/python/rq_worker.py "$@"

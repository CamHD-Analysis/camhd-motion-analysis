#!/bin/bash

echo "launch_worker.sh"

while [ ! -f /output/CamHD_motion_metadata/README.md ]; do
  echo "Could not find output directory /output/CamHD_motion_metadata"
  sleep 5
done

/code/camhd_motion_analysis/python/rq_worker.py "$@"

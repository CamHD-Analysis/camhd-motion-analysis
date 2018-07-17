#!/bin/bash

echo "launch_worker.sh"


## Check for presence of properly mounted CamHD_motion_metadata before
## proceeding
if [ ! -f $OUTPUT_DIR/README.md ]; then
  echo "Could not find output directory $OUTPUT_DIR"
  exit
fi

/code/camhd-motion-analysis/python/rq_worker.py "$@"

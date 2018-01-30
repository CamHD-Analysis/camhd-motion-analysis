#!/bin/bash

echo "launch_worker.sh"

if [ ! -f $OUTPUT_DIR/README.md ]; then
  echo "Could not find output directory $OUTPUT_DIR"
  exit
fi

/code/camhd_motion_analysis/python/rq_worker.py "$@"

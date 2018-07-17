#!/bin/bash

echo "launch_worker.sh"


DATA_DIR=${OUTPUT_DIR:-"/output/CamHD_motion_metadata"}

## Check for presence of properly mounted CamHD_motion_metadata before
## proceeding
echo "Using CamHD_motion_metadata mounted at ${DATA_DIR}"

if [ ! -f $DATA_DIR/README.md ]; then
  echo "Could not find output directory $DATA_DIR"
  exit
fi

if [ ! -z "${DELAY_START}" ]; then
  echo "Delaying start for ${DELAY_START} seconds"
  sleep ${DELAY_START}
fi

/code/camhd-motion-analysis/python/rq_worker.py "$@"

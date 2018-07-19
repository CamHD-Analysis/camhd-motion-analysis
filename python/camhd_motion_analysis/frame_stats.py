from __future__ import absolute_import, unicode_literals
from .celery import app

import tempfile
import subprocess
import json
import pathlib
import logging
import re

from pathlib import Path
import os.path

# frame_stats_path = "/home/aaron/workspace/CamHD_analysis/fips-deploy/camhd-motion-analysis/linux-make-debug/frame_stats",

@app.task
def frame_stats( path, frame,
                lazycache = "http://cache.camhd.science/v1/org/oceanobservatories/rawdata/files" ):

    fips_path = Path(__file__).resolve().parent.parent.parent
    fips_path = fips_path / "cpp/fips"

    if not fips_path.is_file():
        logging.fatal("Unable to find fips at %s" % fips_path)

    logging.info("Processing %s frame %d" % (path, frame))

    args = [ fips_path, "run", "frame_stats", "--",
                                           "--frame", str(frame),
                                           "--lazycache-url", lazycache,
                                            path ]
    procout = subprocess.run( args,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            encoding='utf8' )

    #print("Stderr: %s" % procout.stderr)

    # Trim any extra output from fips which has yellow formatting
    doc = re.sub('\x1b\[.*?m.*\x1b\[.*?m', '', procout.stdout)

    try:
        # Read the JSON from stdout
        results = json.JSONDecoder().decode( doc )

        #TODO: Check output quality here

        return results
    except json.JSONDecodeError as ex:
        logging.warning('Error decoding JSON from the application at row=%d, col=%d: %s\n%s' %
                        (ex.lineno, ex.colno, ex.msg, ex.doc) )
        return {}

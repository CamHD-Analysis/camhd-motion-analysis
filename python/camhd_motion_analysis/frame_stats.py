
import tempfile
import subprocess
import json
import pathlib
import logging

def frame_stats( path, frame,
                frame_stats_path = "/home/aaron/workspace/CamHD_analysis/fips-deploy/camhd-motion-analysis/linux-make-debug/frame_stats",
                host = "http://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files" ):

    logging.info("Processing %s frame %d" % (path, frame))
    procout = subprocess.run( [frame_stats_path,
                                           "--frame", str(frame),
                                           "--lazycache-url", host,
                                            path ],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            encoding='utf8' )

    logging.info("Stderr: %s" % procout.stderr)

    try:
        # Read the JSON from stdout
        results = json.loads( procout.stdout )

        #TODO: Check output quality here

        return results
    except json.JSONDecodeError as ex:
        logging.error('Error decoding JSON from the application:\n %s', ex.doc)
        return procout.stdout

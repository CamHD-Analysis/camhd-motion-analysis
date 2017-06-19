
import tempfile
import subprocess
import json
import pathlib
import logging

def frame_stats( path, frame,
                frame_stats_path = "frame_stats",
                host = "http://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files" ):

    with tempfile.NamedTemporaryFile() as t:
        logging.info("Processing %s frame %d" % (path, frame))
        procout = subprocess.run( [frame_stats_path,
                                               "-o", t.name,
                                               "--frame", str(frame),
                                               "--lazycache-url", host,
                                                path ],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                encoding='utf8' )

        logging.info("Stdout: %s" % procout.stdout)
        logging.info("Stderr: %s" % procout.stderr)

        try:
            # Read the JSON from stdout
            results = json.load( t )

            #TODO: Check output quality here

            return results
        except json.JSONDecodeError:
            logging.error('Error decoding JSON from the application')
            return procout.stdout


import tempfile
import subprocess
import json
import pathlib

def frame_stats( path, start,
                end = -1,
                stride = 10,
                frame_stats_path = "frame_stats",
                host = "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files" ):
    if end < 0: end = start+1

    with tempfile.NamedTemporaryFile() as t:
        procout = subprocess.run( [frame_stats_path,
                                               "-o", t.name,
                                               "--start-at", str(start),
                                               "--stop-at", str(end),
                                               "--stride", str(stride),
                                               "--host", host,
                                                path ],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                encoding='utf8' )

        try:
            # Read the JSON from stdout
            results = json.load( t )

            #TODO: Check output quality here

            return results
        except json.JSONDecodeError:
            print("Stdout: ", procout.stdout)
            print("Stderr: ", procout.stderr)
            return procout.stdout

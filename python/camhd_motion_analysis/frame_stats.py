
import tempfile
import subprocess
import json
import pathlib

def frame_stats( path, start, end = -1, stride = 10,
                frame_stats_path = str(pathlib.Path(__file__).parents[2] / "build-Debug/bin/frame_stats") ):
    if end < 0: end = start+1

    print(frame_stats_path)

    with tempfile.NamedTemporaryFile() as t:
        procout = subprocess.run( [frame_stats_path,
                                               "-o", t.name,
                                               "--start-at", str(start),
                                               "--stop-at", str(end),
                                               "--stride", str(stride),
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
            return procout.stdout

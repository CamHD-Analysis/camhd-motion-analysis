#!/usr/bin/env python

import argparse
import os.path
import json

# import pycamhd.lazycache as pycamhd
#
# import paths

import camhd_motion_analysis as ma
import pycamhd.lazycache as pycamhd

from dask import compute, delayed
import dask.threaded


#
# metadata_repo = "/tmp"
# data_filename = "/CAMHDA301-20160101T000000Z_optical_flow.json"
#
DEFAULT_STRIDE = 10

def process_file( mov_path, output_path, num_threads=1, start = 1, stop =-1, stride = DEFAULT_STRIDE ):

    if stop < 0:
        repo = pycamhd.lazycache()
        movie_info = repo.get_metadata( url=mov_path )
        stop = movie_info['NumFrames']


    print("Processing %s from %d to %d by %d in %d threads" % (mov_path, start, stop, stride, num_threads))
    frames = range( start, stop, stride )

    if( num_threads > 1 ):
        values = [delayed(ma.frame_stats)(mov_path,f) for f in frames]
        results = compute(*values, get=dask.threaded.get)

        joutput = results[0]
        for i in range(1, len(results)):
            joutput["frame_stats"].extend(results[i]["frame_stats"])
    else:
        joutput = [ma.frame_stats(mov_path, f) for f in frames]


    with open(output_path,'w') as f:
        json.dump(joutput, f, indent=2)


    return joutput

#!/usr/bin/env python

import argparse
import os.path
import json

import logging

# import pycamhd.lazycache as pycamhd
#
# import paths

import camhd_motion_analysis as ma
import pycamhd.lazycache as pycamhd

from dask import compute, delayed
import dask.threaded
import dask.multiprocessing

DEFAULT_STRIDE = 10

DEFAULT_LAZYCACHE_HOST = "http://camhd-app-dev-nocache.appspot.com/v1/org/oceanobservatories/rawdata/files"
#LAZYCACHE_HOST = "http://ursine:9080/v1/org/oceanobservatories/rawdata/files"

def process_file( mov_path, output_path,
                num_threads=1, start = 1, stop =-1,
                lazycache_url = DEFAULT_LAZYCACHE_HOST,
                stride = DEFAULT_STRIDE ):

    logging.info('Using Lazycache at %s' % lazycache_url)
    logging.info("Processing %s" % mov_path)

    if stop < 0:
        repo = pycamhd.lazycache( lazycache_url  )
        movie_info = repo.get_metadata( url=mov_path )
        stop = movie_info['NumFrames']

    if os.path.isfile( output_path ):
        logging.warning("File %s exists, skipping" % output_path )
        return

    logging.info("Processing %s from %d to %d by %d in %d threads" % (mov_path, start, stop, stride, num_threads))
    frames = range( start, stop, stride )

    if( num_threads > 1 ):
        values = [delayed(ma.frame_stats)(mov_path,f, host=lazycache_url ) for f in frames]
        results = compute(*values, get=dask.threaded.get)

        joutput = results[0]
        for i in range(1, len(results)):
            joutput["frame_stats"].extend(results[i]["frame_stats"])
    else:
        joutput = [ma.frame_stats(mov_path, f, host=lazycache_url ) for f in frames]

    os.makedirs( os.path.dirname( output_path ), exist_ok = True )

    logging.info("Saving results to %s" % output_path )

    with open(output_path,'w') as f:
        json.dump(joutput, f, indent=2)


    return joutput

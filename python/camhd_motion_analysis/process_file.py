#!/usr/bin/env python

import argparse
import os.path
import json

import logging
import platform

import cpuinfo
from datetime import datetime

# import pycamhd.lazycache as pycamhd
#
# import paths

import camhd_motion_analysis as ma
import pycamhd.lazycache as pycamhd

from dask import compute, delayed
import dask.threaded
import dask.multiprocessing

import time

DEFAULT_STRIDE = 10

DEFAULT_LAZYCACHE_HOST = "http://camhd-app-dev-nocache.appspot.com/v1/org/oceanobservatories/rawdata/files"
#LAZYCACHE_HOST = "http://ursine:9080/v1/org/oceanobservatories/rawdata/files"

def process_file( mov_path, output_path,
                num_threads=1, start = 1, stop =-1,
                lazycache_url = DEFAULT_LAZYCACHE_HOST,
                stride = DEFAULT_STRIDE ):

    startTime = datetime.now()

    logging.info('Using Lazycache at %s' % lazycache_url)
    logging.info("Processing %s" % mov_path)

    if stop < 0:
        logging.info("Querying lazycache at %s for movie length" % lazycache_url )
        repo = pycamhd.lazycache( lazycache_url )
        movie_info = repo.get_metadata( url=mov_path, timeout=120  )
        stop = movie_info['NumFrames']

    # if os.path.isfile( output_path ):
    #     logging.warning("File %s exists, skipping" % output_path )
    #     return

    logging.info("Processing %s from %d to %d by %d in %d threads" % (mov_path, start, stop, stride, num_threads))
    frames = range( start, stop, stride )

    start_time = time.time()

    if( num_threads > 1 ):
        values = [delayed(ma.frame_stats)(mov_path,f, host=lazycache_url ) for f in frames]
        results = compute(*values, get=dask.threaded.get)
    else:
        results = [ma.frame_stats(mov_path,f, host=lazycache_url) for f in frames ]

    end_time = time.time()

    joutput = results[0]
    for i in range(1, len(results)):
        joutput["frameStats"].extend(results[i]["frameStats"])



    endTime = datetime.now();

    joutput["contents"]["performance"] = {"timing": "1.0", "hostinfo": "1.1" }

    info = cpuinfo.get_cpu_info()

    joutput["performance"] = {"timing": { "elapsedSeconds":  (end_time - start_time),
                                            "startTime" : str(startTime),
                                            "endTime" : str(endTime) },
                                          "hostinfo" : {"hostname": platform.node(),
                                          "cpu":  info['brand'] } }


    os.makedirs( os.path.dirname( output_path ), exist_ok = True )

    logging.info("Saving results to %s" % output_path )

    with open(output_path,'w') as f:
        json.dump(joutput, f, indent=2)


    return joutput

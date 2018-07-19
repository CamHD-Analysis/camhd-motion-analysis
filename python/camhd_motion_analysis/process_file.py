#!/usr/bin/env python

import argparse
import os.path
import json
import re

import io

import logging
import platform

import cpuinfo
from datetime import datetime

import minio
from decouple import config
from urllib.parse import urlparse

from minio import Minio
from minio.error import ResponseError

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
DEFAULT_LAZYCACHE_HOST = "http://cache.camhd.science/v1/org/oceanobservatories/rawdata/files"

def process_file( mov_path, destination=config('OUTPUT_DEST',"s3://minio/CamHD_motion_metadata/"),
                num_threads=1, start = 1, stop =-1,
                lazycache_url = DEFAULT_LAZYCACHE_HOST,
                stride = DEFAULT_STRIDE ):

    startTime = datetime.now()

    logging.info('Using Lazycache at %s' % lazycache_url)
    logging.info("Processing %s" % mov_path)

    if stop < 0:
        logging.info("Querying lazycache at %s for movie length" % lazycache_url )
        repo = pycamhd.lazycache( lazycache_url )
        movie_info = repo.get_metadata( url=mov_path, timeout=120 )
        stop = movie_info['NumFrames']

    logging.info("Processing %s from %d to %d by %d in %d threads" % (mov_path, start, stop, stride, num_threads))
    frames = range( start, stop, stride )

    start_time = time.time()

    if( num_threads > 1 ):
        values = [delayed(ma.frame_stats)(mov_path,f, lazycache=lazycache_url ) for f in frames]
        results = compute(*values, get=dask.threaded.get)
    else:
        results = [ma.frame_stats(mov_path,f, lazycache=lazycache_url) for f in frames ]

    end_time = time.time()

    joutput = None
    for result in results:
        if "frameStats" in result:
            if not joutput:
                joutput = result
            else:
                joutput["frameStats"].extend(result["frameStats"])

    endTime = datetime.now();

    joutput["contents"]["performance"] = {"timing": "1.0", "hostinfo": "1.1" }

    info = cpuinfo.get_cpu_info()

    joutput["performance"] = {"timing": { "elapsedSeconds":  (end_time - start_time),
                                          "startTime" : str(startTime),
                                          "endTime" : str(endTime) },
                                          "hostinfo" : {"hostname": platform.node(),
                                          "cpu":  info['brand'] } }


    if destination:

        o = urlparse(destination)

        if o.scheme == 's3' or o.scheme=="http" or o.scheme=="https":
            logging.info("Saving to S3 location %s" % destination)

            client = Minio(o.netloc,
                            access_key=config('S3_ACCESS_KEY','camhd'),
                            secret_key=config('S3_SECRET_KEY','camhdcamhd'),
                            secure=False)

            jbytes = bytes(json.dumps(joutput, indent=2), encoding='utf-8')

            ## This should handle repeated slashes in path...
            split_path = re.split(r'/+', o.path.lstrip("/"))
            split_path = list(filter(len, split_path))

            bucket = split_path[0]
            path = '/'.join(split_path[1:])

            logging.info("Saving to bucket %s as %s" % (bucket,path))

            if not client.bucket_exists(bucket):
                client.make_bucket(bucket)

            with io.BytesIO(jbytes) as data:
                client.put_object(bucket,path,
                                    data, len(jbytes) )

        else:
            ## Assume it's a path
            logging.info("Saving to path %s" % o.path)

            os.makedirs( os.path.dirname( o.path ), exist_ok = True )

            logging.info("Saving results to %s" % o.path )

            with open(o.path,'w') as f:
                json.dump(joutput, f, indent=2)


    return joutput

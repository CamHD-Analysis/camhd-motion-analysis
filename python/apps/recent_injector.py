#!/usr/bin/env python3

# Sample execution:
#  python/apps/recent_injector.py --days 30 --log INFO --threads 16


import argparse
from datetime import datetime, timedelta

import os.path
import re

import logging
from decouple import config

import pycamhd.lazycache as pycamhd
from urllib.parse import urlparse

from minio import Minio
from minio.error import ResponseError,NoSuchBucket,NoSuchKey

import camhd_motion_analysis as ma

DEFAULT_STRIDE = 10
DEFAULT_STOP = -1
DEFAULT_START = -1

parser = argparse.ArgumentParser(description='Run rq_job_injector on recent days')

parser.add_argument('--days', metavar='j', type=int, nargs='?', default=1,
                    help='Number of days to consider')

parser.add_argument('--dry-run', dest='dryrun', action='store_true', help='Dry run')

parser.add_argument('--log', metavar='log', nargs='?', default='WARNING',
                    help='Logging level')

parser.add_argument('--threads', metavar='j', type=int, nargs='?', default=1,
                    help='Number of threads to run with dask')

parser.add_argument('--count', metavar='j', type=int, nargs='?',
                    help='')

parser.add_argument('--stride', metavar='s', type=int, nargs='?',
                    default=config("FRAME_STRIDE",DEFAULT_STRIDE),
                    help='Stride for frame stats')

parser.add_argument('--start', metavar='s', type=int, nargs='?',
                    default=config("START_FRAME",DEFAULT_START),
                    help='Frame to start at')

parser.add_argument('--stop', metavar='s', type=int, nargs='?',
                    default=config("STOP_FRAME",DEFAULT_STOP),
                    help='Frame to stop at')

parser.add_argument('--destination', dest='destination', metavar='o', nargs='?',
                    default=config("DESTINATION","s3://minio:9000/camhd-motion-metadata"),
                    help='Path for output')

parser.add_argument('--lazycache-url', dest='lazycache',
                    default=config("LAZYCACHE_URL", "http://lazycache:8080/v1/org/oceanobservatories/rawdata/files/"),
                    help='Lazycache URL to pass to jobs')


args = parser.parse_args()

logging.basicConfig(level=args.log.upper())


start_date = datetime.now() - timedelta(args.days)


## Bad DRY with rq_job_injector

def iterate_path(path):
    repo = pycamhd.lazycache(args.lazycache)

    logging.info("Querying path %s from %s" % (path, args.lazycache))

    dir_info = repo.get_dir(path)

    if not dir_info:
        logging.warning("Unable to query directory: %s" % (path))
        return []

    outfiles = []
    for f in dir_info['Files']:
        if re.search('\.mov$', f):
            outfiles.append(path + f)

    for d in dir_info['Directories']:
        outfiles += iterate_path(path + d + "/")

    return outfiles

def daterange(start_date, end_date):
    # Need to add one day to be inclusive of current day
    for n in range(int ((timedelta(1) + end_date - start_date).days)):
        yield start_date + timedelta(n)

num_queued = 0
for single_date in daterange(start_date, datetime.now()):
    logging.info("Checking %s" % single_date.strftime("%Y-%m-%d"))

    infiles = iterate_path( single_date.strftime("/RS03ASHS/PN03B/06-CAMHDA301/%Y/%m/%d/") )

    # Generate infile->outfile pairs:
    if len(infiles) == 0:
        continue

    for infile in infiles:
        destination = os.path.splitext(args.destination + infile)[0] + "_optical_flow.json"

        ## Need to reduce DRY with process_file.py

        o = urlparse(destination)

        ## TODO.  This could be done more efficently with minio.list_objects
        if o.scheme == 's3' or o.scheme=="http" or o.scheme=="https":
            logging.info("Checking S3 location %s" % destination)

            client = Minio(o.netloc,
                            access_key=config('S3_ACCESS_KEY','camhd'),
                            secret_key=config('S3_SECRET_KEY','camhdcamhd'),
                            secure=False)

            split_path = o.path.lstrip("/").split("/")

            bucket = split_path[0]
            path = '/'.join(split_path[1:])

            logging.debug("   bucket: %s,   path: %s" % (bucket,path))

            try:
                client.stat_object(bucket,path)
                logging.info("%s exists in S3, skipping" % path)
                continue
            except NoSuchKey as err:
                # Distinguish between connection error and file not present
                logging.info("Can't find path %s in bucket %s, processing" % (path,bucket))
                pass

        else:
            ## Assume it's a path

            if os.path.isfile(destination):
                logging.warning("Output file %s exists, skipping" % destination)
                continue

        num_queued += 1
        if args.count and num_queued > args.count:
            logging.info("Reach maximum count of %d, stopping" % args.count)
            break

        logging.info("Injecting job to process %s, Saving results to %s" % (infile, destination))

        if args.dryrun==True:
            continue

        job = ma.process_file.delay( infile,
                        destination=destination,
                        lazycache_url=args.lazycache,
                        num_threads=args.threads,
                        stride=args.stride,
                        start=args.start,
                        stop=args.stop)

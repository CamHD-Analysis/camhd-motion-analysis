#!/usr/bin/env python3

import argparse
from datetime import datetime, timedelta

from redis import Redis
from rq import Queue

import os.path
import re

import logging

import pycamhd.lazycache as pycamhd

import camhd_motion_analysis as ma

DEFAULT_STRIDE = 10

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
                    default=DEFAULT_STRIDE,
                    help='Stride for frame stats')

parser.add_argument('--output-dir', dest='outdir', metavar='o', nargs='?',
                    default=os.environ.get("OUTPUT_DIR","/output/CamHD_motion_metadata"),
                    help='File for output')

parser.add_argument('--lazycache-url', dest='lazycache',
                    default=os.environ.get("RQ_LAZYCACHE_URL", None),
                    help='Lazycache URL to pass to jobs')

parser.add_argument('--redis-url', dest='redis',
                    default=os.environ.get("RQ_REDIS_URL", "redis://localhost:6379/"),
                    help='URL to Redis server')


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

q = Queue(connection=Redis.from_url(args.redis))


num_queued = 0
for single_date in daterange(start_date, datetime.now()):
    logging.info("Checking %s" % single_date.strftime("%Y-%m-%d"))

    infiles = iterate_path( single_date.strftime("/RS03ASHS/PN03B/06-CAMHDA301/%Y/%m/%d/") )

    # Generate infile->outfile pairs:
    if len(infiles) == 0:
        continue

    for infile in infiles:
        outfile = os.path.splitext(args.outdir + infile)[0] + "_optical_flow.json"
        logging.info("Processing %s, Saving results to %s" % (infile, outfile))

        if os.path.isfile(outfile):
            logging.warning("Output file %s exists, skipping" % outfile)
            continue

        num_queued += 1
        if num_queued > 0 and num_queued > args.count:
            logging.info("Reach maximum count of %d, stopping" % args.count)
            break;


        if args.dryrun==True:
            continue

        job = q.enqueue(ma.process_file,
                        infile,
                        outfile,
                        lazycache_url=args.lazycache,
                        num_threads=args.threads,
                        stride=args.stride,
                        start=-1,
                        stop=-1,
                        timeout='24h',
    		            result_ttl=3600)

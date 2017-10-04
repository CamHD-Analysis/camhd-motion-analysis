#!/usr/bin/env python3

import argparse

from redis import Redis
from rq import Queue

import os.path
import re

import logging

import pycamhd.lazycache as pycamhd

import camhd_motion_analysis as ma

DEFAULT_STRIDE=10

parser = argparse.ArgumentParser(description='Process a file using frame_stats.')

parser.add_argument('input', metavar='N', nargs='+',
                    help='Path to process')

parser.add_argument('--threads', metavar='j', type=int, nargs='?', default=1,
                    help='Number of threads to run with dask')

parser.add_argument('--start', type=int, nargs='?', default=1,
                    help='Frame to start')

parser.add_argument('--stop', type=int, nargs='?', default=-1,
                    help='Frame to stop')

parser.add_argument('--stride', metavar='s', type=int, nargs='?', default=DEFAULT_STRIDE,
                    help='Stride for frame stats')

parser.add_argument('--output-file', dest='outfile', metavar='o', nargs='?', default="output.json",
                    help='File for output')

parser.add_argument('--output-dir', dest='outdir', metavar='o', nargs='?', default=os.environ.get("OUTPUT_DIR","/output/CamHD_motion_metadata"),
                    help='File for output')

parser.add_argument('--dry-run', dest='dryrun', action='store_true', help='Dry run')

parser.add_argument('--client-lazycache-url', dest='clientlazycache', default=os.environ.get("RQ_LAZYCACHE_URL", None),
                    help='Lazycache URL to use for job injection')

parser.add_argument('--lazycache-url', dest='lazycache', default=os.environ.get("RQ_LAZYCACHE_URL", None),
                    help='Lazycache URL to pass to jobs')

parser.add_argument('--redis-url', dest='redis', default=os.environ.get("RQ_REDIS_URL", "redis://localhost:6379/"),
                    help='URL to Redis server')

parser.add_argument('--log', metavar='log', nargs='?', default='WARNING',
                    help='Logging level')

args = parser.parse_args()

logging.basicConfig( level=args.log.upper() )


## If output-file is specified, then input must be a single file
## otherwise output-dir must be set
#  and len(args.input) == 1:
#     filepairs = [ [args.input[0], args.outfile] ]
# else:
    ## First convert input args to list of files


# Use --lazycache-url to set the Lazycache for the _jobs_
# Also use it for this script _unless_ --client-lazycache-url
# is set
client_lazycache = args.lazycache
if args.clientlazycache != None:
    client_lazycache = args.clientlazycache


def iterate_path( path ):
    repo = pycamhd.lazycache(client_lazycache)
    dir_info = repo.get_dir(path)

    if not dir_info:
        return []

    outfiles = []
    for f in dir_info['Files']:
        if re.search('\.mov$', f ):
            outfiles.append( path + f)

    for d in dir_info['Directories']:
        outfiles += iterate_path( path + d + "/" )

    return outfiles

infiles = []
for f in args.input:
    if re.search('\.mov$', f ):
        infiles.append(f)
    else:
        logging.info("Iterating on %s" % f)
        infiles += iterate_path( f )

## Generate infile->outfile pairs:
if len(infiles) == 0:
    exit

q = Queue( connection=Redis.from_url(args.redis) )
for infile in infiles:
    outfile = os.path.splitext(args.outdir + infile)[0] + "_optical_flow.json"
    logging.info("Processing %s, Saving results to %s" % (infile, outfile) )

    if os.path.isfile( outfile ):
        logging.warning("Output file %s exists, skipping" % outfile )
        continue

    if args.dryrun == True:
        continue

    job = q.enqueue( ma.process_file,
                    infile,
                    outfile,
                    lazycache_url = args.lazycache,
                    num_threads=args.threads,
                    stride=args.stride,
                    start=args.start,
                    stop=args.stop,
                    timeout='24h',
		            result_ttl=3600)

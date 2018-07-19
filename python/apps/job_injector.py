#!/usr/bin/env python3

import argparse

import os.path
import re

import logging

from decouple import config
import pycamhd.lazycache as pycamhd

import camhd_motion_analysis as ma

DEFAULT_STRIDE = 10

parser = argparse.ArgumentParser(description='Inject an RQ job.')

parser.add_argument('input', metavar='N', nargs='+',
                    help='Path to process')

parser.add_argument('--threads', metavar='j', type=int, nargs='?', default=1,
                    help='Number of threads to run in worker')

parser.add_argument('--start', type=int, nargs='?', default=1,
                    help='Frame to start')

parser.add_argument('--stop', type=int, nargs='?', default=-1,
                    help='Frame to stop')

parser.add_argument('--stride', metavar='s', type=int, nargs='?',
                    default=DEFAULT_STRIDE,
                    help='Stride for frame stats')

parser.add_argument('--destination', dest='destination', metavar='o', nargs='?',
                    default=config("DESTINATION","s3://minio:9000/camhd-motion-metadata"),
                    help='Path for output')

parser.add_argument('--dry-run', dest='dryrun', action='store_true', help='Dry run')

parser.add_argument('--run-local', dest='runlocal', action='store_true',
                        help='Run job locally, not with celery')

parser.add_argument('--lazycache-url', dest='lazycache',
                    default=config("LAZYCACHE_URL", "http://lazycache:8080/v1/org/oceanobservatories/rawdata/files/"),
                    help='Lazycache URL to pass to jobs')

parser.add_argument('--log', metavar='log', nargs='?', default='WARNING',
                    help='Logging level')

args = parser.parse_args()

logging.basicConfig(level=args.log.upper())

# Use --lazycache-url to set the Lazycache for the _jobs_
# Also use it for this script _unless_ --client-lazycache-url
# is set

def iterate_path(path):
    repo = pycamhd.lazycache(args.lazycache)
    dir_info = repo.get_dir(path)

    if not dir_info:
        return []

    outfiles = []
    for f in dir_info['Files']:
        if re.search('\.mov$', f):
            outfiles.append(path + f)

    for d in dir_info['Directories']:
        outfiles += iterate_path(path + d + "/")

    return outfiles


infiles = []
for f in args.input:
    if re.search('\.mov$', f):
        infiles.append(f)
    else:
        logging.info("Iterating on %s" % f)
        infiles += iterate_path(f)

# Generate infile->outfile pairs:
if len(infiles) == 0:
    exit

for infile in infiles:
    outfile = os.path.splitext(args.destination + infile)[0] + "_optical_flow.json"
    logging.info("Processing %s, Saving results to %s" % (infile, outfile))

    if os.path.isfile(outfile):
        logging.warning("Output file %s exists, skipping" % outfile)
        continue

    if args.dryrun==True:
        continue

    if args.runlocal:

        job = ma.process_file( infile,
                                outfile,
                                lazycache_url=args.lazycache,
                                num_threads=args.threads,
                                stride=args.stride,
                                start=args.start,
                                stop=args.stop)

    else:

        job = ma.process_file.delay( infile,
                                    outfile,
                                    lazycache_url=args.lazycache,
                                    num_threads=args.threads,
                                    stride=args.stride,
                                    start=args.start,
                                    stop=args.stop)

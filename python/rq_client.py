#!/usr/bin/env python

import argparse

from redis import Redis
from rq import Queue

import os.path

import camhd_motion_analysis as ma


DEFAULT_STRIDE=10

parser = argparse.ArgumentParser(description='Process a file using frame_stats.')

parser.add_argument('input', metavar='N', nargs='+',
                    help='Path to process')

parser.add_argument('--threads', metavar='j', type=int, nargs='?', default=1,
                    help='Number of threads to run with dask')

parser.add_argument('--stride', metavar='s', type=int, nargs='?', default=DEFAULT_STRIDE,
                    help='Stride for frame stats')

parser.add_argument('--output-file', dest='output', metavar='o', nargs='?', default="",
                    help='File for output')

args = parser.parse_args()


#mov_path = args.'/RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov'

infile = args.input[0]
outfile = args.output
if len(outfile) == 0:
    outfile = os.path.basename( infile )
    outfile = os.path.splitext( outfile )[0] + "_optical_flow.json"

print("Processing %s, Saving results to %s" % (infile, outfile) )

q = Queue(connection=Redis())
job = q.enqueue( ma.process_file,
                    infile,
                    outfile,
                    num_threads=args.threads,
                    stride=args.stride )

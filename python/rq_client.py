#!/usr/bin/env python

import argparse

from redis import Redis
from rq import Queue

import os.path

import camhd_motion_analysis as ma


DEFAULT_STRIDE=10

parser = argparse.ArgumentParser(description='Process a file using frame_stats.')

parser.add_argument('threads', metavar='j', type=int, nargs='?', default=1,
                    help='Number of threads to run with dask')

parser.add_argument('stride', metavar='s', type=int, nargs='?', default=DEFAULT_STRIDE,
                    help='Stride for frame stats')

parser.add_argument('savedir', metavar='o', nargs='?', default='.',
                    help='Directory to save complete file')

args = parser.parse_args()


mov_path = '/RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov'

data_filename = os.path.basename(mov_path)
data_filename = os.path.splitext( data_filename )[0] + "_optical_flow.json"
metadata_repo = args.savedir

outfile = metadata_repo + "/" + data_filename
print("Saving results to %s" % outfile)

q = Queue(connection=Redis())
job = q.enqueue( ma.process_file, mov_path, outfile,
                    num_threads=args.threads,
                    start=5000, stop=5010,
                    stride=args.stride )

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
        values = [delayed(frame_stats)(mov_path,f) for f in frames]
        results = compute(*values, get=dask.threaded.get)

        joutput = results[0]
        for i in range(1, len(results)):
            joutput["frame_stats"].extend(results[i]["frame_stats"])
    else:
        joutput = [ma.frame_stats(mov_path, f) for f in frames]

    print(joutput)

    return joutput


if __name__ == "__main__":
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


    joutput = process_file( mov_path, outfile, num_threads = args.threads )

    with open(outfile,'w') as f:
        json.dump(joutput, f, indent=2)

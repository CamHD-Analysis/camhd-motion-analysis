#!/usr/bin/env python3

import json
import logging
import argparse


import camhd_motion_analysis as ma

parser = argparse.ArgumentParser(description='Process a file using frame_stats.')

parser.add_argument('input', metavar='paths', nargs='+',
                    help='*_optical_flow.json file to analyze')

parser.add_argument('--output', dest='output', metavar='o', nargs='?', default="output.json",
                    help='File for output')

parser.add_argument('--log', metavar='log', nargs='?', default='WARNING',
                    help='Logging level')

args = parser.parse_args()

logging.basicConfig( level=args.log.upper() )


infile = args.input[0]
with open(infile) as data_file:
    json_out = ma.region_analysis( data_file )

    print("Writing to %s" % args.output )

    with open( args.output, 'w' ) as outfile:
        json.dump( json_out, outfile, indent = 4)

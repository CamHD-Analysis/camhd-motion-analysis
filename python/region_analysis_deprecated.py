#!/usr/bin/env python3

import json
import logging
import argparse


import camhd_motion_analysis as ma

parser = argparse.ArgumentParser(description='Perform region analysis on an optical flow file.')

parser.add_argument('input', metavar='optical_flow_file', nargs='+',
                    help='*_optical_flow.json file to analyze')

parser.add_argument('--output', dest='output', metavar='output_file', nargs='?', default="output.json",
                    help='File for output')

parser.add_argument('--log', metavar='log', nargs='?', default='INFO',
                    help='Logging level')

args = parser.parse_args()

logging.basicConfig( level=args.log.upper() )


infile = args.input[0]
with open(infile) as data_file:
    ma.region_analysis( data_file, outfile=args.output )

    # logging.info("Writing JSON results to %s" % args.output )
    # with open( args.output, 'w' ) as outfile:
    #     json.dump( json_out, outfile, indent = 4)

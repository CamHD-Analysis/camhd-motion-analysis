#!/usr/bin/env python3

import numpy as np
import json
import time
from os import path
import pandas as pd

import argparse

import camhd_motion_analysis as ma

parser = argparse.ArgumentParser(description='Process a file using frame_stats.')

parser.add_argument('input', metavar='N', nargs='+',
                    help='*_optical_flow.json file to analyze')

parser.add_argument('--output', dest='output', metavar='o', nargs='?', default="output.json",
                    help='File for output')



args = parser.parse_args()


infile = args.input[0]
with open(infile) as data_file:
    j = json.load(data_file)


frame_num = [ f["frame_number"] for f in j["frame_stats"]]
similarities = [ f["similarity"] for f in j["frame_stats"] ]

stats = pd.DataFrame(similarities, index=frame_num).sort_index()

## Extract only valid
valid = stats[ stats.valid == True ]

# Break the similarity structure out into columns
#similarity = pd.DataFrame.from_records( valid.similarity, valid.index )

## Convert center columns to center_x, center_y
valid = pd.concat( [valid.center.apply( pd.Series ), valid.drop('center', axis=1)], axis=1) \
            .rename( columns={ 0: 'center_x', 1: 'center_y '} )

valid = pd.concat( [valid.similarity.apply( pd.Series ), valid.drop('similarity', axis=1)], axis=1) \
            .rename( columns={ 0: 'scale', 1: 'theta', 2: 'trans_x', 3: 'trans_y'} )

valid['trans'] = valid.trans_x**2 + valid.trans_y**2


stable = valid.loc[lambda df: df.trans < 100].loc[ lambda df: (df.scale-1).abs() < 0.01 ]

def contiguous_region(series, delta = 10):
    series['dt'] = series.index.to_series().diff(1).fillna(0)
    series['block'] = (series.index.to_series().diff(1) > (delta*1.01) ).cumsum()
    #print(series)

    blocks = series.groupby('block')
    #print(blocks.groups)

    static_regions = []
    for name,group in blocks:
        static_regions += [ [ np.asscalar(group.index.min()), np.asscalar(group.index.max()) ] ]

    ## Drop regions which are too short

    static_regions = [r for r in static_regions if (r[1]-r[0] > 1)]

    return static_regions


def classify_regions( valid, static ):

    regions = []
    for r in static:
        regions.append( {"bounds": r, "type": "static", "stats": calc_stats(valid, r) } )

    ## Now fill in the regions
    for i in range(0, len(static)-1):
        start = static[i][1]+10;    ## Hm, 10 is hard coded right now...
        end = static[i+1][0];

        bounds = [start,end]

        region = analyze_bounds( valid, bounds )
        if region: regions.append( region )

    return regions


def calc_stats( series, bounds ):
    subset = series.iloc[lambda df: df.index >= bounds[0]].iloc[lambda df: df.index < bounds[1]]

    return {
        "scale_mean": subset.scale.mean(),
        "tx_mean": subset.trans_x.mean(),
        "ty_mean": subset.trans_y.mean(),
        "size": np.asscalar(subset.size)
    }


def analyze_bounds( series, bounds ):
    ## heuristics for now
    #print(bounds)

    stats = calc_stats( series, bounds )

    out = {"bounds": bounds,
           "type": "unknown",
          "stat": stats}

    if stats["size"] < 2:
        out["type"] = "short"
        return out

    if stats["scale_mean"] > 1.05: out["type"] = "zoom_in"
    if stats["scale_mean"] < 0.95: out["type"] = "zoom_out"

    ## Ugliness
    if stats["tx_mean"] > 10:
        if stats["ty_mean"] > 10:
            out["type"] = "NW"
        elif stats["ty_mean"] < -10:
            out["type"] = "SW"
        else:
            out["type"] = "W"
    elif stats["tx_mean"] < -10:
        if stats["ty_mean"] > 10:
            out["type"] = "NE"
        elif stats["ty_mean"] < -10:
            out["type"] = "SE"
        else:
            out["type"] = "E"
    elif stats["ty_mean"] > 10:
            out["type"] = "N"
    elif stats["ty_mean"] < -10:
            out["type"] = "S"


    return out


stable_regions = contiguous_region( stable )
classify = classify_regions( valid, stable_regions )

classify.sort(key=lambda x: x["bounds"][0])


#regions_filename = metadata_repo + path.splitext(data_filename)[0] + '_regions.json'

## Write metainformation
json_out = { 'movie': j['movie'],
             'contents': { 'regions': '1.0' },
            'regions': classify }

print("Writing to %s" % args.output )

with open( args.output, 'w' ) as outfile:
    json.dump( json_out, outfile, indent = 4)

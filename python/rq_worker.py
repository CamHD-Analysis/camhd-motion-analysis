#!/usr/bin/env python3

import sys
import os
import argparse
import logging

from redis import Redis
from rq import Connection, Worker


# There's actually (basically) nothing specific about the worker, though the
# Python environment needs to be configured properly.

# Preload libraries
#import camhd_motion_analysis as ma

parser = argparse.ArgumentParser(description='CamHD RQ Worker.')

parser.add_argument('queues', metavar='N', nargs='*', default='default',
                    help='RQ Queues to listen to')

parser.add_argument('--redis-url', dest='redis', default=os.environ.get("RQ_REDIS_URL", "redis://localhost:6379/"),
                    help='URL to Redis server')

parser.add_argument('--log', metavar='log', nargs='?', default='WARNING',
                    help='Logging level')

args = parser.parse_args()

logging.basicConfig( level=args.log.upper() )

redis_host = args.redis
logging.warning("Connecting to Redis host %s" % redis_host )

conn = Redis.from_url(redis_host)
with Connection(conn):
    w = Worker( args.queues )
    w.work()

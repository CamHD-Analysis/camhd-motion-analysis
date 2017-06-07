#!/usr/bin/env python3

import sys
import os

from redis import Redis
from rq import Connection, Worker


# There's actually (basically) nothing specific about this worker..

# Preload libraries
# import camhd_motion_analysis as ma

redis_host = os.environ.get("REDIS_URL", "redis://localhost:6379/1")

print("Connecting to Redis host %s" % redis_host )

conn = Redis.from_url(redis_host)
with Connection(conn):
    qs = sys.argv[1:] or ['default']

    w = Worker(qs)
    w.work()

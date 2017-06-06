#!/usr/bin/env python3

import sys
from redis import Redis
from rq import Connection, Worker

# Preload libraries
# import camhd_motion_analysis as ma

conn = Redis(host="localhost", port=6379, db=0)
with Connection(conn):
    qs = sys.argv[1:] or ['default']

    w = Worker(qs)
    w.work()


# if __name__ == "__main__":
#     print("Hello from rq_worker.py")

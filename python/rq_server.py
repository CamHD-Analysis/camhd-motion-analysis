#!usr/bin/env python
from redis import redis
from rq import Queue


q = Queue(connection=Redis())

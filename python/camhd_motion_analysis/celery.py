from __future__ import absolute_import, unicode_literals
from celery import Celery
from decouple import config

app = Celery('covis_worker',
             broker=config('CELERY_BROKER', default='amqp://user:bitnami@rabbitmq'),

             backend=config('CELERY_BACKEND', default='rpc://'),
             include=['camhd_motion_analysis.process_file', 'camhd_motion_analysis.frame_stats'])

## Default configuration changes
app.broker_connection_timeout = 30
app.result_expires=3600


if __name__ == '__main__':
    app.start()

#! /bin/bash

sudo service nfs-kernel-server start

sudo docker swarm init

echo "REDIS_PASSWORD=`curl \"http://metadata.google.internal/computeMetadata/v1/project/attributes/rq_redis_password\" -H \"Metadata-Flavor: Google\"`" > /tmp/redis.env
docker run --detach -p 6379:6379 --env-file /tmp/redis.env  bitnami/redis:latest

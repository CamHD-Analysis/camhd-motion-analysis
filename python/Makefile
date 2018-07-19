

worker:
	if [ ! -z "$${DELAY_START}" ]; then echo "Delaying start for $$DELAY_START seconds"; sleep $$DELAY_START; fi; \
	celery -A camhd_motion_analysis worker -l info --concurrency 1 --without-mingle --without-gossip --events

## Use the ENV variable preferentially, otherwise here's a default
CELERY_BROKER ?= amqp://user:bitnami@localhost

flower:
	celery flower -A camhd_motion_analysis --broker=${CELERY_BROKER}

.PHONY: worker flower

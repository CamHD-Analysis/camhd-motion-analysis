# camhd-motion-analysis-deploy

Scripts for scalable deployment of camhd-motion-analysis

Builds three Docker images:

* `amarburg/camhd-worker-base:latest` is a Xenial-based base image with all of the common code
* `amarburg/camhd-worker:latest` builds `camhd_motion_analysis` from a pristine Github checkout
* `amarburg/camhd-worker:test`   copies a local copy of `camhd_motion_analysis` into the Docker image.

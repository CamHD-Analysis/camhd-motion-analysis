# This is a comment
FROM amarburg/lsdslam-dev-host:conan-latest
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

RUN apt-get install --no-install-recommends --yes libcurlpp-dev libcurl4-openssl-dev

WORKDIR /opt/camhd_motion_tracking

ENTRYPOINT ["rake"]
ENV BUILD_DIR build_docker

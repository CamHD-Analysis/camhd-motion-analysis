# This is a comment
FROM amarburg/lsdslam-dev-host:conan-latest
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

WORKDIR /opt/camhd_motion_tracking

RUN pip install --upgrade conan

ENTRYPOINT ["rake"]
ENV BUILD_DIR build_docker

# This is a comment
FROM amarburg/lsdslam-dev-host:conan-latest
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

WORKDIR /opt

RUN pip install --upgrade conan

RUN git clone https://github.com/CamHD-Analysis/camhd_motion_analysis.git
WORKDIR camhd_motion_analysis
RUN rake release:test

ENTRYPOINT ["/opt/camhd_motion_analysis/build-Release/bin/test_frame"]

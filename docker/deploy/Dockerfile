# This is a comment
FROM camhd_motion_analysis_base:latest
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

WORKDIR /root
RUN git clone https://github.com/CamHD-Analysis/camhd_motion_analysis.git

WORKDIR camhd_motion_analysis
RUN git submodule init && git submodule update

## Force conan configuration
RUN conan config set settings_defaults.compiler.libcxx=libstdc++11

ENV VERBOSE 1
RUN rake release:build

VOLUME "/data"

ENTRYPOINT ["/root/camhd_motion_analysis/build-Release/bin/frame_stats"]

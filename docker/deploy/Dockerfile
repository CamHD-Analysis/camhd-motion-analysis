# This is a comment
FROM amarburg/lsdslam-dev-host:conan-latest
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

RUN apt-get install --no-install-recommends --yes libcurlpp-dev libcurl4-openssl-dev libceres-dev

## Resolve a bug in the current xenial package for Ceres
RUN touch /usr/include/ceres/internal/config.h

## Resolve a bug in the current xenial package for curlpp
RUN sed -i s/std::string\(\\\([a-zA-Z\ \\\"]*\\\)\)/\\\1/ /usr/include/curlpp/Option.inl

## Updated pip
RUN pip install -U conan

RUN git clone https://github.com/CamHD-Analysis/camhd_motion_analysis.git

WORKDIR camhd_motion_analysis
RUN git submodule init && git submodule update

## Force conan configuration
RUN conan config set settings_defaults.compiler.libcxx=libstdc++11

ENV VERBOSE 1
RUN rake release:build

ENTRYPOINT ["/root/camhd_motion_analysis/build-Release/bin/test_frame"]

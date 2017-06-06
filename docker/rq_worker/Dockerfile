#
FROM ubuntu:yakkety
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

RUN apt-get update && apt-get install --no-install-recommends --yes libcurlpp-dev libcurl4-openssl-dev \
          libopencv-dev rake g++ cmake python3.6 libpython3.6-dev python3-pip python-setuptools git \
          libboost-filesystem-dev libeigen3-dev libceres-dev

RUN pip3 install setuptools
RUN pip3 install --upgrade pip
RUN pip3 install wheel conan rq pillow redis numpy

## Resolve a bug in the current xenial (and yakkety?) package for Ceres
RUN touch /usr/include/ceres/internal/config.h

## Resolve a bug in the current xenial (and yakkety?) package for curlpp
RUN sed -i s/std::string\(\\\([a-zA-Z\ \\\"]*\\\)\)/\\\1/ /usr/include/curlpp/Option.inl

ADD . /code/camhd_motion_analysis
WORKDIR /code/camhd_motion_analysis

## Initial conan configuration
RUN conan config set settings_defaults.compiler.libcxx=libstdc++11

ENV BUILD_DIR docker-Release
ENV VERBOSE 1
RUN rake release:build
RUN cd $BUILD_DIR && make clean

## Install pycamhd
WORKDIR /code
RUN git clone https://github.com/CamHD-Analysis/pycamhd-lazycache.git
WORKDIR /code/pycamhd-lazycache
RUN pip3 install .

## Todo:  move up.  These are all dependencies of pycamhd-lazycache (?)
RUN pip3 install dask cytoolz numpy

WORKDIR /code/camhd_motion_analysis/python
VOLUME /output/CamHD_motion_metadata
ENTRYPOINT ["./rq_worker.py"]

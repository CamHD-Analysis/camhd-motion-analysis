#
FROM ubuntu:yakkety
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

RUN apt-get update && apt-get install --no-install-recommends --yes libcurlpp-dev libcurl4-openssl-dev \
          libopencv-dev rake g++ cmake python-pip

## TODO.   Consolidate with above when you have time to do a massive
RUN apt-get install --no-install-recommends --yes python-setuptools git

RUN pip install --upgrade pip
RUN pip install wheel
RUN pip install conan rq pillow

## TODO.   Consolidate with above when you have time to do a massive
RUN apt-get install --no-install-recommends --yes libboost-filesystem-dev libeigen3-dev

ADD . /code
WORKDIR /code

RUN ls -al

## Force conan configuration
RUN conan config set settings_defaults.compiler.libcxx=libstdc++11

ENV BUILD_DIR docker-Release
ENV VERBOSE 1
RUN rake release:build

ENTRYPOINT ["python3 python/rq_worker.py"]

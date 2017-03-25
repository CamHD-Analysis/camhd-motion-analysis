# This is a comment
FROM amarburg/lsdslam-dev-host:conan-latest
MAINTAINER Aaron Marburg <amarburg@apl.washington.edu>

RUN apt-get install --no-install-recommends --yes libcurlpp-dev libcurl4-openssl-dev

WORKDIR /opt
RUN git clone https://github.com/CamHD-Analysis/camhd_motion_analysis.git

WORKDIR camhd_motion_analysis
RUN git submodule init && git submodule update
ENV VERBOSE 1
RUN sed -i s/std::string\(\\\([a-zA-Z\ \\\"]*\\\)\)/\\\1/ /usr/include/curlpp/Option.inl
RUN conan config set settings_defaults.compiler.libcxx=libstdc++11
RUN rake release:test

ENTRYPOINT ["/opt/camhd_motion_analysis/build-Release/bin/test_frame"]

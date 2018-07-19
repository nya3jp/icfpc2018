FROM debian:stretch

RUN apt-get update && \
    apt-get install -y wget build-essential unzip && \
    apt-get clean

RUN cd /tmp && \
    wget https://github.com/bazelbuild/bazel/releases/download/0.15.2/bazel-0.15.2-installer-linux-x86_64.sh && \
    chmod +x bazel-0.15.2-installer-linux-x86_64.sh && \
    ./bazel-0.15.2-installer-linux-x86_64.sh && \
    rm bazel-0.15.2-installer-linux-x86_64.sh

RUN apt-get update && \
    apt-get install -y cmake pkg-config zip g++ zlib1g-dev unzip python && \
    apt-get clean

RUN useradd build && \
    mkdir -p /home/build && \
    chown build:build /home/build
USER build
WORKDIR /build

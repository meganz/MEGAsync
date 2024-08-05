# MEGA clang-format Linter
# mega-docker.artifactory.developers.mega.co.nz:8443/clang-format-desktop:latest

FROM ubuntu:22.04

ENV DEBCONF_NOWARNINGS=yes
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y git clang-format && \
    useradd clang-format -d /var/lib/clang-format -m -s /bin/bash

USER clang-format

WORKDIR /var/lib/clang-format

ARG VARIANT=buster
FROM mcr.microsoft.com/vscode/devcontainers/base:${VARIANT}

RUN apt-get update \
    && apt-get -y install --no-install-recommends bzip2 cmake make ninja-build \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /toolchain
RUN wget -qO- https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 | tar -xj
ENV PATH $PATH:/toolchain/gcc-arm-none-eabi-9-2020-q2-update/bin
WORKDIR /gnse

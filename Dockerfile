# syntax=docker/dockerfile:1

FROM alpine:latest
LABEL org.opencontainers.image.authors="Vladimir Inshakov <markoni48@yandex.ru>" \
      org.opencontainers.image.title="FreeRTOS app buider for Posix/Linux" \
      version="0.1b" \
      description="Posix/Linux FreeRTOS Simulator application builder"
#   org.opencontainers.image.documentation="URL..."

# Установка базовых пакетов linux
RUN apk update && apk add --no-cache bash

# Установка компиляторов Си, Си++ (gdb, llvm, clang, lldb)
RUN apk add --no-cache build-base libpcap-dev libbsd libbsd-dev git cmake

# Копирование исходников FreeRTOS с портированным Posix GCC
WORKDIR /home
RUN git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git

# What the container should run when it is started.
ENTRYPOINT [ "/bin/bash", "-l" ]

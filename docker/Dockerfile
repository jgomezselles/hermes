FROM ghcr.io/jgomezselles/hermes_base:0.0.3 as builder

COPY . /code

WORKDIR /code/build
RUN  cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4 hermes && install src/hermes /usr/local/bin
RUN mkdir libs && cp /lib64/libboost_thread.so.1.78.0 /lib64/libboost_system.so.1.78.0 /lib64/libboost_filesystem.so.1.78.0 \
    /lib64/libcrypto.so.3 /lib64/libstdc++.so.6 /lib64/libm.so.6 /lib64/libgcc_s.so.1 \
    /lib64/libc.so.6 /lib64/libboost_atomic.so.1.78.0 /lib64/libz.so.1 /lib64/libssl.so.3 libs/

FROM quay.io/fedora/fedora:38
WORKDIR /hermes
COPY --from=builder /code/build/src/hermes /usr/local/bin/hermes
COPY --from=builder /code/build/libs/ /lib64/
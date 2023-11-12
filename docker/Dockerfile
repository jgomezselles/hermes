FROM ghcr.io/jgomezselles/hermes_base:0.0.4 as builder

COPY . /code

WORKDIR /code/build
RUN  cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4 hermes
RUN mkdir libs && cp /lib64/libopentelemetry_proto.so /lib64/libboost_thread.so.1.78.0 /lib64/libboost_system.so.1.78.0 \
    /lib64/libboost_filesystem.so.1.78.0 /lib64/libcurl.so.4 /lib64/libprotobuf.so.30 /lib64/libssl.so.3 \
    /lib64/libcrypto.so.3 /lib64/libstdc++.so.6 /lib64/libm.so.6 /lib64/libgcc_s.so.1 \
    /lib64/libc.so.6 /lib64/libboost_atomic.so.1.78.0 /lib64/libnghttp2.so.14 /lib64/libidn2.so.0 \
    /lib64/libssh.so.4 /lib64/libpsl.so.5 /lib64/libgssapi_krb5.so.2 /lib64/libldap.so.2 /lib64/liblber.so.2 \
    /lib64/libbrotlidec.so.1 /lib64/libz.so.1 /lib64/libunistring.so.2 /lib64/libkrb5.so.3 /lib64/libk5crypto.so.3 \
    /lib64/libcom_err.so.2 /lib64/libkrb5support.so.0 /lib64/libkeyutils.so.1 /lib64/libresolv.so.2 \
    /lib64/libevent-2.1.so.7 /lib64/libsasl2.so.3 /lib64/libbrotlicommon.so.1 /lib64/libpcre2-8.so.0 libs/

FROM quay.io/fedora/fedora:38
WORKDIR /hermes
COPY --from=builder /code/build/src/hermes /usr/local/bin/hermes
COPY --from=builder /code/build/libs/ /lib64/

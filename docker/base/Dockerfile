FROM quay.io/fedora/fedora:38

RUN dnf -y install cmake gcc-c++ libcurl-devel wget boost-devel bzip2 \
        openssl-devel openssl ca-certificates protobuf-devel git

WORKDIR /code/build

RUN wget https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz -O gtest.tar.gz && \
    tar xvf gtest.tar.gz && cd googletest-1.13.0/ && cmake . && make -j4 install && cd .. && rm -r * && set +x

RUN set -x && wget https://github.com/nghttp2/nghttp2/releases/download/v1.45.1/nghttp2-1.45.1.tar.bz2 && \
    tar xf nghttp2* && cd nghttp2-1.45.1/ && ./configure --enable-asio-lib --disable-shared --enable-python-bindings=no && \
    make -j4 install && cd .. && rm -rf * && set +x

RUN set -x && wget https://github.com/Tencent/rapidjson/archive/4b3d7c2.tar.gz -O rapidjson.tar.gz && \
    tar xf rapidjson.tar.gz && cd rapidjson-4b3d7c2f42142f10b888e580c515f60ca98e2ee9/ && \
    cmake -DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF \
                -DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=0 -DRAPIDJSON_SCHEMA_USE_STDREGEX=1 . && \
    make install && cd .. && rm -rf * && set +x

RUN set -x && wget https://github.com/open-telemetry/opentelemetry-cpp/archive/refs/tags/v1.10.0.tar.gz -O otel.tar.gz && \
    tar xvf otel.tar.gz && cd opentelemetry* && cmake -DBUILD_TESTING=OFF -DWITH_OTLP_HTTP=ON . && \
    make -j4 install && cp /usr/local/lib64/libopentelemetry_proto.so /lib64/ && cd .. && rm -r * && set +x
FROM alpine:latest

RUN apk add build-base cmake wget tar linux-headers openssl-dev libev-dev openssl-libs-static

WORKDIR /code/build

RUN set -x && wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz && \
    tar xvf boost* && cd boost* && ./bootstrap.sh && ./b2 -j4 install && cd .. && rm -rf * && set +x

RUN set -x && wget  https://github.com/google/googletest/archive/2fe3bd9.tar.gz -O gtest.tar.gz && \
    tar xvf gtest.tar.gz && cd googletest* && cmake . && make -j4 install && cd .. && rm -rf * && set +x

RUN set -x && wget https://github.com/nghttp2/nghttp2/releases/download/v1.45.1/nghttp2-1.45.1.tar.bz2 && \
    tar xf nghttp2* && cd nghttp2* && ./configure --enable-asio-lib --disable-shared --enable-python-bindings=no && \
    make -j4 install && cd .. && rm -rf * && set +x

RUN set -x && wget https://github.com/Tencent/rapidjson/archive/4b3d7c2.tar.gz -O rapidjson.tar.gz && \
    tar xf rapidjson.tar.gz && cd rapid* && \
    cmake -DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF \
                -DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=0 -DRAPIDJSON_SCHEMA_USE_STDREGEX=1 . && \
    make install && cd .. && rm -rf * && set +x
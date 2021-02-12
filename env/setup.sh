#!/bin/bash
set -x
: "
    This script installs all the required dependencies needed to
    compile Hermes, which are:
    * boost 1.67
    * nghttp2 1.39.2
    * rapidjson 4b3d7c2
    * gtest 2fe3bd9

    This script is intended for Ubuntu 18.04
"

WORKDIR="/tmp"

BOOST_DIR="boost_1_67_0"
BOOST_PACKAGE="${BOOST_DIR}_rc2.tar.gz"
BOOST_URL="https://dl.bintray.com/boostorg/release/1.67.0/source/${BOOST_PACKAGE}"

GTEST="googletest"
GTEST_URL="https://github.com/google/${GTEST}"
GTEST_VERSION="2fe3bd9"

RAPIDJSON="rapidjson"
RAPIDJSON_URL="https://github.com/Tencent/${RAPIDJSON}"
RAPIDJSON_VERSION="4b3d7c2"

NGHTTP2="nghttp2-1.39.2"
NGHTTP2_PACKAGE="${NGHTTP2}.tar.bz2"
NGHTTP2_URL="https://github.com/nghttp2/nghttp2/releases/download/v1.39.2/${NGHTTP2_PACKAGE}"

CORES="$(nproc)"

function _check_if_debian()
{
    test "$(grep -ni "debian" /etc/os-release)"
}

function _install_pre_requisites()
{
    if ! _check_if_debian; then
        echo "THE SYSTEM IS NOT SUPPORTED"
        return 1
    fi
    apt-get update && apt-get install -y \
        cmake make gcc g++ git wget \
        g++ make binutils autoconf automake autotools-dev libtool pkg-config \
        zlib1g-dev libcunit1-dev libssl-dev libxml2-dev libev-dev libevent-dev libjansson-dev \
        libc-ares-dev libjemalloc-dev libsystemd-dev \
        cython python3-dev python-setuptools \
        clang-format
}

function _install_boost()
{
    cd "${WORKDIR}" && \
        wget "${BOOST_URL}" && tar xvf "${BOOST_PACKAGE}" && cd "${BOOST_DIR}" && \
            ./bootstrap.sh && ./b2 -j"${CORES}" install
    cd "${WORKDIR}" && rm -rf "${BOOST_DIR} ${BOOST_PACKAGE}"
}

function _install_gtest()
{
    cd "${WORKDIR}" || exit
    git clone "${GTEST_URL}" && cd "${GTEST}" && \
    git checkout "${GTEST_VERSION}" && \
    cmake . && make -j"${CORES}" install
    cd "${WORKDIR}" && rm -rf "${GTEST}"
}

function _install_rapidjson()
{
    cd "${WORKDIR}" && \
        git clone "${RAPIDJSON_URL}" && cd "${RAPIDJSON}" && \
        git checkout "${RAPIDJSON_VERSION}" && \
        cmake -DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF \
                -DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=0 -DRAPIDJSON_SCHEMA_USE_STDREGEX=1 . && \
        make -j"${CORES}" install
    cd "${WORKDIR}" && rm -rf "${RAPIDJSON}"

}

function _install_nghttp2()
{
    cd "${WORKDIR}" && \
        wget "${NGHTTP2_URL}" && \
        tar xvf "${NGHTTP2_PACKAGE}" && cd "${NGHTTP2}" && \
        ./configure --enable-asio-lib --enable-app --disable-shared  && \
        make -j"${CORES}" install
    cd "${WORKDIR}" && rm -rf "${NGHTTP2_PACKAGE}" "${NGHTTP2}"
}

if "$(_install_pre_requisites)" == 1; then
    echo "PRE-REQUISITES NOT INSTALLED, CAN'T CONTINUE"
    exit
fi

_install_boost
_install_gtest
_install_rapidjson
_install_nghttp2

ldconfig

set +x

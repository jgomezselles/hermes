#!/bin/bash

BUILD_IMAGE_NAME="hermes_build:0.1.0"
COMPILED_EXECUTABLE="$1"
shift
docker run --rm -u "$(id -u):$(id -g)" -v "${PWD}":/code --entrypoint "${COMPILED_EXECUTABLE}" "${BUILD_IMAGE_NAME}" "$@"

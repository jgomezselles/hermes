#!/bin/bash

BUILD_IMAGE_NAME="hermes_build:0.1.0"

docker run --rm -u "$(id -u):$(id -g)" -v "${PWD}":/code "${BUILD_IMAGE_NAME}" "$@"

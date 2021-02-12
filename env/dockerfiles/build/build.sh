#!/bin/bash

# Formatting phase
find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\|h\|c\|hh\)' -exec clang-format --verbose -style=file -i {} \;
# Building phase
mkdir -p build && cd build && cmake .. && make -j"$(nproc)"

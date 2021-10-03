# Dev information

This document presents some useful tips to build and test hermes.

## Building & testing

Use hermes' docker images to build and run the project. Information on how to generate
hermes and hermes_base docker images under [docker folder](/docker/README.md).

Alternatively, you could always do it in your local machine by reproducing the
steps in both Dockerfiles.

In any case, the supported way of building and testing the hermes is via Dockerfiles.
This is because in that way, builds are reproducible and maintaineble.

Build a development image by running, from the root of this repository:

```bash
docker run --rm -it -v "${PWD}":/code ghcr.io/jgomezselles/hermes_base:0.0.2
```

In this way, you will be sharing your local copy of the repository with the development
docker container. This container will have all needed dependencies by hermes to be compiled
and tested, and you can still lkaunch your favorite IDE from the outside.

It will automatically enter in `/code/build` as working directory. To build both the code and
unitary tests, run:

```
cmake ..
make -j<N>
```

Tests will be located under `/code/build/ut/unit-tests`, which is an executable you can run.

## Format

To format the code, `clang-format` is used (currently version 7), to run `clang-format`you can either install it in
your machine or use `hermes_base`. In both cases, you may run (from the repository root):

```bash
find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\|h\|c\|hh\)' -exec clang-format -style=file -i {} \;
```
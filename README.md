hermes
======================

[[_TOC_]]

## General information

### Building

Use hermes' docker images to build and run the project.
Please check on how to generate the docker environment images under [Environment Documentation](/env/README.md).

Alternatively, you could always do it in your local machine by reproducing the
steps in the scripts and Dockerfiles.

In any case, the supported way of building and testing the hermes is via Dockerfiles.
This is because in that way, builds are reproducible and maintaineble.

### Format

To format the code we use `clang-format` (currently version 7), to run `clang-format`you can either install it in
your machine or use `hermes_base`. In both cases, you may run (from the repository root):

```bash
find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\|h\|c\|hh\)' -exec clang-format -style=file -i {} \;
```

In any case, `build.sh` runs the formatting directly.
### Run Hermes

Just run the generated binary and see the options available

```bash
docker run --rm hermes_prod -h
```

While developing you can use:
```bash
./devrun.sh build/src/hermes -h
```

### Testing

After generating or downloading the images following the [Environment Documentation](/env/README.md),
use the following command to compile and/or run the unitary tests.

### Compile unit tests
```bash
./build.sh
```
This script is just a simple oneliner utility to build the test executable from
an existing `hermes_build` image and places the results under a new folder called `build`.

### Run unit tests

After compiling, simply run:

```bash
./devrun.sh build/ut/unit-test
```
Which, again, is a utility which mounts the previously generated `build` directory
and runs the test inside the `hermes_build` image.

To run the unit tests with different configurations (if you are working on a
specific test, for instance) run a command like the following and start playing
around:

```bash
docker run --rm -it --entrypoint=bash -v "${PWD}":/code hermes_build
```
The compiled `unit-test` executable can be found under `/code/build/ut/unit-test`

### Repository structure

Check the [docs](/docs/) for more information about the project

# License

The Software implemented in this repository is distributed under MIT license,
as stated in the [LICENSE](/LICENSE) file. It makes use of some
[boost C++ library](https://www.boost.org/doc/libs/1_67_0/) functionalities,
which have their own [license](https://www.boost.org/LICENSE_1_0.txt).
Hermes core functionality is based on [nghttp2](https://nghttp2.org/), and
json handling is done with [rapidjson](https://rapidjson.org/), both under
MIT license.

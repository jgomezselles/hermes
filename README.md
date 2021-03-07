# What is hermes?

Hermes is an http/2 traffic generator written in C++ able to send multiplexed
requests over one connection at a given rate.

It also measures and calculates some statistics, validates response codes and
handles user-defined traffic sequences, a.k.a. scripts.

Because of the nature of the project itself, it has a minimal footprint impact,
and has proven to be reliable: traffic is generated in a steady pace, without
unwanted traffic bursts. This is achieved by using
[boost async io_context](https://www.boost.org/doc/libs/develop/doc/html/boost_asio/reference/io_context.html)
utility and thread synchronization strategies.

It is based in the [nghttp2](https://nghttp2.org/) implementation, and provides
a different functionality than [h2load](https://nghttp2.org/documentation/h2load.1.html),
also written in C++ by the nghttp2 creator, because hermes allows to use traffic scripts,
traffic is injected at a pre-defined rate, and validates the results.

Hermes aims to be a low footprint and easy to use but high-performance general use tool.

# How can you use it?

Hermes docker image and chart will be available soon so you may integrate it in
your regular helm chart deployments by just adding a few artifacts.

## hermes integration (work in progress)
The idea is simple:

* Add it to your requirements:
    * Repo: (coming soon)
    * Name: hermes
    * Version: 0.1.0
    * Image: (coming soon)

* Reference hermes in your chart under .Values.hermes.image repository and tag.
* Define a config map for your traffic definition (script) and reference it
in your values with the name you gave it under `.Values.hermes.script.cm`

But what is inside that ConfigMap? Your [traffic script](doc/traffic_script.md) definition.

## Executing hermes

You may take a look to Hermes help by just typing `./hermes -h`:

```bash
hermes: C++ Traffic Generator. Usage:  hermes [options]

options:
       -r <rate>      Requests/second ( Default: 10 )

       -t <time>      Time to run traffic (s) ( Default: 60 )

       -p <period>    Print and save statistics every <period> (s) ( Default: 10 )

       -f <path>      Path with the traffic json definition ( Default: /etc/scripts/traffic.json )

       -s             Show schema for json traffic definition.

       -o <file>      Output file for statistics( Default: hermes.out )

       -h             This help.

```

At the end of the day a typical use is:

```bash
./hermes -r2400 -p1 -t3600
```

Hermes results, console and file outputs are explained [here](doc/hermes_output.md).

# Dev information

## Building

Use hermes' docker images to build and run the project. Information on how to generate
hermes and hermes_base docker images under [docker folder](/docker/README.md).

Alternatively, you could always do it in your local machine by reproducing the
steps in both Dockerfiles.

In any case, the supported way of building and testing the hermes is via Dockerfiles.
This is because in that way, builds are reproducible and maintaineble.

## Format

To format the code, `clang-format` is used (currently version 7), to run `clang-format`you can either install it in
your machine or use `hermes_base`. In both cases, you may run (from the repository root):

```bash
find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\|h\|c\|hh\)' -exec clang-format -style=file -i {} \;
```

## Run Hermes

Just run the generated binary and see the options available

```bash
docker run --rm jgomezselles/hermes:0.0.1 /hermes/hermes -h
```
## Testing

After generating or downloading the images following the [Docker Documentation](/docker/README.md),
use the following command to compile and/or run the unitary tests.

## Compile unit tests TODO

## Run unit tests TODO

```bash
docker run --rm -it --entrypoint=bash -v "${PWD}":/code jgomezselles/hermes_base:0.0.1
```
The compiled `unit-test` executable can be found under `/code/build/ut/unit-test`

# License

The Software implemented in this repository is distributed under MIT license,
as stated in the [LICENSE](/LICENSE) file. It makes use of some
[boost C++ library](https://www.boost.org/doc/libs/1_67_0/) functionalities,
which have their own [license](https://www.boost.org/LICENSE_1_0.txt).
Hermes core functionality is based on [nghttp2](https://nghttp2.org/), and
json handling is done with [rapidjson](https://rapidjson.org/), both under
MIT license. Tests are done with [Googletest](https://github.com/google/googletest),
which is published under [BSD 3-Clause License](https://github.com/google/googletest/blob/master/LICENSE).
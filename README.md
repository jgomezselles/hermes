[![CI](https://github.com/jgomezselles/hermes/actions/workflows/ci.yml/badge.svg?branch=main&event=push)](https://github.com/jgomezselles/hermes/actions/workflows/ci.yml)
[![Coverage Status](https://coveralls.io/repos/github/jgomezselles/hermes/badge.svg?branch=main)](https://coveralls.io/github/jgomezselles/hermes?branch=main)
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

Hermes may be used standalone (check the [releases](https://github.com/jgomezselles/hermes/releases)!),
from a [docker container](#container-to-container-example), or as part of a kubernetes
[deployment](#hermes-in-kubernetes-example).

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

## hermes helm chart integration
The idea is simple:

* Add it to your `Chart.yaml` dependencies.
* Reference hermes in your chart under `.Values.hermes.image` `repository` and `tag`.
* Define a config map for your traffic definition (script) and reference it
in your values with the name you gave it under `.Values.hermes.script.cm`

But what is inside that ConfigMap? Your [traffic script](doc/traffic_script.md) definition.

And that's it!

If you have some doubts, take a look at the [example-hermes](helm/example-hermes)
helm chart, or the following examples. 

# Examples

This repository provides a server-mock implementation which enables you to test and taste
easily how hermes works.

## Container to Container example

1. Build and run a server-mock container by following [these instructions](docker/README.md#server-mock).
2. Attach to a docker container running a `jgomezselles/hermes` image, and create a script file
like the one located in the helm [example](helm/example-hermes/templates/traffic.script.yaml).
3. Run hermes providing the path to your new script:
```bash
./hermes -r2400 -p1 -t3600 -f <path/to/script.json>
```

## hermes in kubernetes example

1. Build (but not run) a server-mock by following [these instructions](docker/README.md#server-mock).
2. Update the example-hermes helm chart dependencies, and install the chart:
```bash
helm dep update helm/example-hermes
helm install example-hermes helm/example-hermes
```
3. Run hermes
```bash
kubectl exec <hermes-pod> -c hermes -- /hermes/hermes -p1 -t10 -r1000
```

> The server-mock prints every received request to the console, so you can check the received requests
> by inspecting the logs: ` kubectl logs <server-mock-pod>`

# Developing & testing hermes

Take a look to hermes dev info [here](doc/dev_info.md).

# License

The Software implemented in this repository is distributed under MIT license,
as stated in the [LICENSE](/LICENSE) file. It makes use of some
[boost C++ library](https://www.boost.org/doc/libs/1_67_0/) functionalities,
which have their own [license](https://www.boost.org/LICENSE_1_0.txt).
Hermes core functionality is based on [nghttp2](https://nghttp2.org/), and
json handling is done with [rapidjson](https://rapidjson.org/), both under
MIT license. Tests are done with [Googletest](https://github.com/google/googletest),
which is published under [BSD 3-Clause License](https://github.com/google/googletest/blob/master/LICENSE).

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

But what is inside that ConfigMap? Your script (traffic) definition.

## Traffic configmap

The script mentioned above contains your traffic definition. It needs to be in
json format, and compliant with its schema definition. You may ask for the
schema by executing:  ./hermes -s  and play around to see which values are
mandatory, and where they lie.

In brief, what you need is (maybe better to look at example or the schema, **an API definition is coming**):

* `dns`: `string` - your server address
* `port`: `string` - your server port
* `timeout`: `integer` - the number of ms to wait until non answered requests are considered to be a timeout error
* `flow`: `array of strings` – the name or id of the messages, in order, that define your traffic. For example: `[“request1”, “request2”]`.
* `messages`: `json object` – the definition of each one of the messages defined in your `flow`. Every element mentioned under flow, must be defined as an object inside this field, named after its id, with the following content:
    * `method`: `string` – Http method for this message (`“POST”`, `“GET”`…)
    * `url`: `string` – The url for your request. Do not start with “/”!
    * `body`: `json object` – The body of your request
    * `response`: `json object` – must contain:
        * `code`: `integer` – the http response code used to consider that the request was successful, once answered
    * `save_from_answer` – `json object`, **Optional**: used to pass information from an answer to a subsequent request containing:
        * `name`: `string` – an id you want to give to the chunk of the response you want to save
        * `path`: `string` – the path where lies the chunk of the answer you want to save from the response
        * `value_type`: `string` – The type of the chunk of answer you want to save (only `string`, `int` or `object` supported right now)
    * `add_from_saved_to_body` – `json object`, **Optional**: used to construct a request based on a previously stored json fragment from a `save_from_answer` object:
        * `name`: `string` – the id you gave to a previous `save_from_answer`
        * `path`: `string` – the path where you want to add the json fragment into the new request
        * `value_type`: `string` – The type of the fragment (only `string`, `int` or `object` supported right now)
* `ranges`: `json object` – **Optional:** Use this feature if you want the scripts to iterate over certain values. Each property you add here with a name (e.g. *"my_prop”*) will be expanded when such name is found under angle braces (e.g. blabla<my_prop>blabla) in both `url` and `body` of each message. Each of those defined names must contain:
    * `min`: `integer` – minimum value of the iteration
    * `max`: `integer` – maximum value of the iteration
 
For a given script, “request2” will be never sent before “request1” has been
answered. If a new request is needed to be sent before that happens, a new
script is initialized. When ranges are defined, a new value of the range is
taken for every initialized script.

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

## Understanding the output

Once you execute hermes, you will find this in your console:
```bash
hermes-66547c85b6-55vl9:/hermes # ./hermes -r1 -p1 -t12
Rate is 1req/s
Sending a request every 1e+06us
Time (s)      Sent/s    Recv/s        RT (ms)     minRT (ms)     maxRT (ms)           Sent        Success         Errors       Timeouts
Connected to test-server:8080
1.0              1.0       1.0          4.264          3.148          5.776              2              2              0              0
2.0              1.0       1.0          4.264          3.148          5.776              2              2              0              0
3.0              1.0       1.0          4.287          3.148          5.776              3              3              0              0
4.0              1.0       1.0          3.490          1.882          5.776              4              4              0              0
5.0              1.0       1.0          3.468          1.882          5.776              5              5              0              0
6.0              1.0       1.0          3.181          1.882          5.776              6              6              0              0
7.0              1.0       1.0          3.244          1.882          5.776              7              7              0              0
8.0              1.0       1.0          3.077          1.882          5.776              8              8              0              0
9.0              1.0       1.0          3.105          1.882          5.776              9              9              0              0
10.0             1.0       1.0          3.068          1.882          5.776             10             10              0              0
11.0             1.0       1.0          3.104          1.882          5.776             11             11              0              0
12.0             1.0       1.0          3.263          1.882          5.776             12             12              0              0
Execution finished. Printing stats...
Time (s)      Sent/s    Recv/s        RT (ms)     minRT (ms)     maxRT (ms)           Sent        Success         Errors       Timeouts
>>>message1<<<
12.0             0.5       0.5          3.915          3.337          5.776              6              6              0              0
>>>message2<<<
12.0             0.1       0.1          3.148          3.148          3.148              1              1              0              0
>>>message3<<<
12.0             0.2       0.2          2.000          1.882          2.125              2              2              0              0
>>>message4<<<
12.0             0.2       0.2          2.386          2.065          2.757              2              2              0              0
>>>message5<<<
12.0             0.1       0.1          5.653          5.653          5.653              1              1              0              0
>>>Total<<<
12.0             1.0       1.0          3.263          1.882          5.776             12             12              0              0
```

Keep in mind that all printed statistics are cumulative (not partials, so they take
into account all the values of your test) and printed every `p` seconds that you
set in the execution.

Output files are saved by default under “hermes.out.*”, containing:

* `hermes.out.accum` – Cumulative statistics (as the ones you saw in screen)
* `hermes.out.<message-id>` - Cumulative statistics for every message id
* `hermes.out.err` – Cumulative number and type of errors found at print-period “p”
* `hermes.out.partial` – Partial statistics for every print-period “p”.
This means the cumulative statistics between print-periods [pn, pn+1] for all pn
 

> Note: Custom error codes are reported by hermes when having reconnection issues and they are all numbered as 46X. They are not sent by the server, but noted as that when a request could not be sent due to a connection problem (your server most likely went down).


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
docker run --rm hermes_prod:0.1.0 /hermes/hermes -h
```
## Testing

After generating or downloading the images following the [Docker Documentation](/docker/README.md),
use the following command to compile and/or run the unitary tests.

## Compile unit tests TODO

## Run unit tests TODO

```bash
docker run --rm -it --entrypoint=bash -v "${PWD}":/code hermes_base
```
The compiled `unit-test` executable can be found under `/code/build/ut/unit-test`

# License

The Software implemented in this repository is distributed under MIT license,
as stated in the [LICENSE](/LICENSE) file. It makes use of some
[boost C++ library](https://www.boost.org/doc/libs/1_67_0/) functionalities,
which have their own [license](https://www.boost.org/LICENSE_1_0.txt).
Hermes core functionality is based on [nghttp2](https://nghttp2.org/), and
json handling is done with [rapidjson](https://rapidjson.org/), both under
MIT license.
> TODO: Add googletest License

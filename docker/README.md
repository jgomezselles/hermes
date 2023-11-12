# Dockerfiles

This folder contains the dockerfiles used in this project to generate the images.

* `base/Dockerfile`: base fedora docker image with dependencies
* `Dockerfile`: fedora image with hermes inside
* `server-mock/Dockerfile`: golang alpine image with an http/2 (h2c) server

## Building

In order to build the hermes image, it's required that the `base` is already built or downloaded.

To do so, execute, from the root directory of your local repository, the
following commands:

```bash
docker build -f docker/base/Dockerfile . -t ghcr.io/jgomezselles/hermes_base:0.0.4 # You may want to skip this one, and just pull it!
docker build -f docker/Dockerfile . -t ghcr.io/jgomezselles/hermes:<your_favorite_tag>
```

## Server mock

If you want to test against a server mock, the server-mock image provides a minimal http/2 server
without TLS active (h2c) listening on the 8080 port, and serving some URIs.
To build it, just run, from the root of this
repository:

```bash
docker build -f docker/server-mock/Dockerfile . -t ghcr.io/jgomezselles/server-mock:local
```

It will copy and build the code from [h2server.go](../ft/h2server.go), so you may later execute it
by running:

```bash
docker run --rm -it -p 0.0.0.0:8080:8080 ghcr.io/jgomezselles/server-mock:local
```
# Dockerfiles

This folder contains the dockerfiles used in this project to generate the images.

* `base/Dockerfile`: base alpine docker image with dependencies
* `Dockerfile`: alpine image with hermes inside

## Building

In order to build the hermes image, it's required that the `base` is already built or downloaded.

To do so, execute, from the root directory of your local repository, the
following commands:

```bash
docker build -f docker/base/Dockerfile . -t hermes_base:0.0.1
docker build -f docker/Dockerfile . -t hermes:0.0.1
```

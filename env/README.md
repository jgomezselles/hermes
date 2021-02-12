# ENV dir structure and How To

## Structure
```
.
├── dockerfiles
│   ├── base
│   │   └── Dockerfile
│   ├── build
│   │   └── Dockerfile
│   └── prod
│       └── Dockerfile
├── README.md
└── setup.sh
```

* dockerfiles: contains all the related docker configuration and build files
    * base: base docker image for all the Hermes images
    * build: build image for Hermes
    * prod: minimal image needed to run Hermes

* `setup.sh`: script that installs all the required dependencies in Ubuntu
18.04 (or higher) OS.

## How to

### Build the images

In order to build the *build* and *prod* images, it's required that the *base*
is already built or downloaded.

To do so, execute, from the root directory of your local repository copy, the
following commands:

```bash
docker build -f env/dockerfiles/base/Dockerfile . -t hermes_base:0.1.0
docker build -f env/dockerfiles/build/Dockerfile . -t hermes_build:0.1.0
docker build -f env/dockerfiles/prod/Dockerfile . -t hermes_prod:0.1.0
```

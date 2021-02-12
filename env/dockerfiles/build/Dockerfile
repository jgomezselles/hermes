FROM hermes_base:0.1.0

COPY env/dockerfiles/build/build.sh /bin/build
WORKDIR /code

RUN chmod +x /bin/build

ENTRYPOINT ["build"]

FROM debian:buster-slim

COPY env/setup.sh /tmp/setup.sh

RUN set -x && \
  chmod +x /tmp/setup.sh && ./tmp/setup.sh && rm -rf /tmp/* && \
  apt-get autoremove -y && \
  set +x

WORKDIR /code

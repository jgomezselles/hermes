FROM jgomezselles/hermes_base:0.0.1 as builder

COPY . /code
WORKDIR /code/build
RUN  cmake .. && make -j4

FROM alpine:latest
COPY --from=builder /code/build/src/hermes /hermes/hermes

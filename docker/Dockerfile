FROM jgomezselles/hermes_base:0.0.2 as builder

COPY . /code

WORKDIR /code/build
RUN  cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4

FROM alpine:latest
COPY --from=builder /code/build/src/hermes /hermes/hermes

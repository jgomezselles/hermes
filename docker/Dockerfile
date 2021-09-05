FROM jgomezselles/hermes_base:0.0.1 as builder

COPY . /code

WORKDIR /code/ut/ssl
RUN ./generateCryptographicMaterial

WORKDIR /code/build
RUN  cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4 \
    && ./ut/unit-test --gtest_filter=-stats_test_extended.PrintAndWrite

FROM alpine:latest
COPY --from=builder /code/build/src/hermes /hermes/hermes

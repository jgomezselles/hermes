FROM hermes_base:0.0.1 as builder

COPY . /code
WORKDIR /code/build
RUN  cmake .. && make -j4

FROM scratch

ENV LD_LIBRARY_PATH /usr/lib:/lib
COPY --from=builder /code/build/src/hermes /hermes/hermes

COPY --from=builder  "/lib/ld-musl-x86_64.so.1" "/lib/"
COPY --from=builder "/usr/lib/libgcc_s.so.1" "/usr/lib/"

CMD ["/hermes/hermes"]
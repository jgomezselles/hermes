FROM hermes_base:0.1.0 as builder

COPY . /code
RUN mkdir /code/build
WORKDIR /code/build
RUN  cmake .. && make -j4

FROM scratch

ENV LD_LIBRARY_PATH /usr/local/lib:/usr/lib:/lib:/usr/lib64:/lib64
COPY --from=builder /code/build/src/hermes /hermes/hermes

COPY --from=builder  "/lib64/ld-linux-x86-64.so.2" "/lib64/"
COPY --from=builder ["/lib/x86_64-linux-gnu/libdl.so.2", \
                     "/lib/x86_64-linux-gnu/libpthread.so.0", \
                    "/lib/x86_64-linux-gnu/libm.so.6", \
                    "/lib/x86_64-linux-gnu/libgcc_s.so.1", \
                    "/lib/x86_64-linux-gnu/libc.so.6", \
                    "/lib/x86_64-linux-gnu/librt.so.1", \
                     "/lib/x86_64-linux-gnu/"]

COPY --from=builder ["/usr/local/lib/libboost_thread.so.1.67.0", \
                    "/usr/local/lib/libboost_filesystem.so.1.67.0", \
                    "/usr/local/lib/libboost_system.so.1.67.0", \
                    "/usr/local/lib/libboost_regex.so.1.67.0", \
                    "/usr/local/lib/"]

COPY --from=builder ["/usr/lib/x86_64-linux-gnu/libssl.so.1.1", \
                    "/usr/lib/x86_64-linux-gnu/libstdc++.so.6", \
                    "/usr/lib/x86_64-linux-gnu/libcrypto.so.1.1", \
                    "/usr/lib/x86_64-linux-gnu/libicudata.so.63", \
                    "/usr/lib/x86_64-linux-gnu/libicui18n.so.63", \
                    "/usr/lib/x86_64-linux-gnu/libicuuc.so.63", \
                    "/usr/lib/x86_64-linux-gnu/"]

CMD ["/hermes/hermes"]
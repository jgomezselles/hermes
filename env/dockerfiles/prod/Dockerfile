FROM hermes_build:0.1.0 as builder

COPY . /code
WORKDIR /code

RUN build && cp build/src/hermes /bin/hermes && rm -rf build/*

FROM debian:buster-slim

COPY --from=builder /bin/hermes /bin/hermes

COPY --from=builder ["/usr/local/lib/libboost_thread.so.1.67.0", \
                    "/usr/local/lib/libboost_filesystem.so.1.67.0", \
                    "/usr/local/lib/libboost_system.so.1.67.0", \
                    "/usr/local/lib/libboost_regex.so.1.67.0", \
                    "/usr/lib/x86_64-linux-gnu/libssl.so.1.1", \
                    "/usr/lib/x86_64-linux-gnu/libstdc++.so.6", \
                    "/usr/lib/x86_64-linux-gnu/libcrypto.so.1.1", \
                    "/usr/lib/x86_64-linux-gnu/libicudata.so.63", \
                    "/usr/lib/x86_64-linux-gnu/libicui18n.so.63", \
                    "/usr/lib/x86_64-linux-gnu/libicuuc.so.63", \
                    "/usr/lib/"]

RUN ldconfig

RUN useradd user
USER user

ENTRYPOINT ["hermes"]
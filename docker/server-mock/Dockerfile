FROM golang:alpine AS builder

COPY ft/h2server /go/src

WORKDIR /go/src

RUN go mod init hermes/h2server && go get golang.org/x/net/http2 \
  go.opentelemetry.io/otel \
  go.opentelemetry.io/otel/exporters/stdout/stdoutmetric \
  go.opentelemetry.io/otel/exporters/stdout/stdouttrace \
  go.opentelemetry.io/otel/exporters/otlp/otlptrace/otlptracehttp \
  go.opentelemetry.io/otel/exporters/otlp/otlplog/otlploghttp \
  go.opentelemetry.io/otel/exporters/stdout/stdoutlog \
  go.opentelemetry.io/otel/sdk/log \
  go.opentelemetry.io/otel/log/global \
  go.opentelemetry.io/otel/propagation \
  go.opentelemetry.io/otel/sdk/metric \
  go.opentelemetry.io/otel/sdk/resource \
  go.opentelemetry.io/otel/sdk/trace \
  go.opentelemetry.io/contrib/instrumentation/net/http/otelhttp \
  go.opentelemetry.io/contrib/bridges/otelslog

RUN go build
FROM alpine
COPY --from=builder /go/src/h2server /bin

CMD ["/bin/h2server"]

#ENV OTEL_EXPORTER_OTLP_PROTOCOL=http/protobuf
#ENV OTEL_EXPORTER_OTLP_ENDPOINT=http://otelcol:4318

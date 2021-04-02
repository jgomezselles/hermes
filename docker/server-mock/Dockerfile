FROM golang:alpine

COPY ft/h2server /go/src

WORKDIR /go/src

RUN go mod init hermes/h2server && go get golang.org/x/net/http2 && go build h2server.go

CMD ["/go/src/h2server"]
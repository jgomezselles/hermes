#!/bin/sh

mkdir -p /usr/local/share/ca-certificates

if test -f "/usr/local/share/ca-certificates/localhost.crt"; then
    if test -f "/usr/local/share/ca-certificates/localhost.key"; then
        echo "Certificates found for tests. Skipping their generation."
        return 0
    fi
fi

# Create the CA
openssl genrsa -out rootCA.key 4096
openssl req -x509 -new -nodes -key rootCA.key -subj "/C=ES/ST=es" -sha256 -days 1024 -out rootCA.crt

# Make the container trust the CA
cp rootCA.crt  /usr/local/share/ca-certificates/rootCA.crt
update-ca-certificates

# Generate localhost cryptographic material
openssl genrsa -out localhost.key 2048
openssl req -new -sha256 -key localhost.key -subj "/C=ES/ST=es/O=MyOrg, Inc./CN=localhost" -out localhost.csr
# Verify the csr's content
openssl req -in localhost.csr -noout -text

# Generate the certificate using the mydomain csr and key along with the CA Root key
openssl x509 -req -in localhost.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out localhost.crt -days 500 -sha256

# Verify the certificate's content
openssl x509 -in localhost.crt -text -noout

mv *.crt *.key *.srl *.csr /usr/local/share/ca-certificates/
package main

import (
	"fmt"
	"net/http"

	"golang.org/x/net/http2"
	"golang.org/x/net/http2/h2c"
)

func defaultHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Request received for Unregistered URI:", r.RequestURI, "Method:", r.Method)
	w.WriteHeader(400)
}

func exampleHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Request received. URI:", r.RequestURI, "Method:", r.Method)
	w.WriteHeader(200)
}

func main() {
	h2s := http2.Server{}
	mux := http.NewServeMux()

	mux.HandleFunc("/url/example/path/", exampleHandler)
	mux.HandleFunc("/url/example/path", exampleHandler)
	mux.HandleFunc("/", defaultHandler)

	server := http.Server{
		Addr:    "0.0.0.0:8080",
		Handler: h2c.NewHandler(mux, &h2s),
	}

	fmt.Printf("Listening [0.0.0.0:8080]...\n")
	server.ListenAndServe()
}

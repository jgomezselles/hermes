package main

import (
	"fmt"
	"net/http"
	"time"

	"golang.org/x/net/http2"
	"golang.org/x/net/http2/h2c"
)

func defaultHandler(w http.ResponseWriter, r *http.Request) {
	time.Sleep(8 * time.Millisecond)
	fmt.Println("Request received for Unregistered URI:", r.RequestURI, "Method:", r.Method)
	w.WriteHeader(400)
}

func exampleHandler(w http.ResponseWriter, r *http.Request) {
	time.Sleep(2 * time.Millisecond)
	fmt.Println("Request received. URI:", r.RequestURI, "Method:", r.Method)
	w.WriteHeader(200)
}

func timeoutHandler(w http.ResponseWriter, r *http.Request) {
	//This will trigger a timeout or error from time to time
	time.Sleep(1960 * time.Millisecond)
	fmt.Println("Request received. URI:", r.RequestURI, "Method:", r.Method)
	w.WriteHeader(500)
}

func errorHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Request received. URI:", r.RequestURI, "Method:", r.Method)
	w.WriteHeader(500)
}

func main() {
	h2s := http2.Server{}
	mux := http.NewServeMux()

	mux.HandleFunc("/url/error/", errorHandler)
	mux.HandleFunc("/url/error", errorHandler)
	mux.HandleFunc("/url/timeout/", timeoutHandler)
	mux.HandleFunc("/url/timeout", timeoutHandler)
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

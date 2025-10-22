package main

import (
	"context"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"go.opentelemetry.io/contrib/instrumentation/net/http/otelhttp"
	"golang.org/x/net/http2"
	"golang.org/x/net/http2/h2c"
)

func logHeaders(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Printf("\n--- %s %s ---\n", r.Method, r.RequestURI)
		for name, values := range r.Header {
			for _, value := range values {
				fmt.Printf("%s: %s\n", name, value)
			}
		}
		next.ServeHTTP(w, r)
	})
}

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
	if err := run(); err != nil {
		log.Fatalln(err)
	}
}

func run() error {
	ctx := context.Background()

	shutdown, err := setupOTelSDK(ctx)
	if err != nil {
		log.Fatalf("initTracer: %v", err)
	}
	defer func() {
		// Ensure spans are flushed on exit.
		_ = shutdown(context.Background())
	}()

	h2s := http2.Server{}
	mux := http.NewServeMux()

	// Your existing handlers.
	mux.HandleFunc("/url/error/", errorHandler)
	mux.HandleFunc("/url/error", errorHandler)
	mux.HandleFunc("/url/timeout/", timeoutHandler)
	mux.HandleFunc("/url/timeout", timeoutHandler)
	mux.HandleFunc("/url/example/path/", exampleHandler)
	mux.HandleFunc("/url/example/path", exampleHandler)
	mux.HandleFunc("/", defaultHandler)

	// Wrap the mux with otelhttp to auto-create server spans, record status, attrs, etc.
	// IMPORTANT: Keep h2c.NewHandler to preserve HTTP/2 (cleartext) support.
	otelMux := otelhttp.NewHandler(
		mux,
		"http.server", // span name for requests that don't match a more specific route
		// You can add options like:
		// otelhttp.WithRouteTag(true),
		// otelhttp.WithSpanNameFormatter(func(operation string, r *http.Request) string { return r.Method + " " + r.URL.Path }),
	)

	server := &http.Server{
		Addr:    "0.0.0.0:8080",
		Handler: h2c.NewHandler(otelMux, &h2s),
	}

	// Graceful shutdown
	go func() {
		fmt.Printf("Listening [0.0.0.0:8080] (HTTP/2 h2c)…\n")
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("ListenAndServe: %v", err)
		}
	}()
	// Wait for SIGINT/SIGTERM
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, os.Interrupt, syscall.SIGTERM)
	<-sig
	_ = server.Shutdown(context.Background())

	return nil
}

{
  description = "Hermes - HTTP/2 traffic generator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        # Use unmodified libnghttp2_asio (it already depends on boost186)
        libnghttp2_asio = pkgs.libnghttp2_asio;

        # Enable HTTP client support in OpenTelemetry SDK
        otel-cpp = pkgs.opentelemetry-cpp.override { enableHttp = true; };

        # Build rapidjson with std::regex for JSON Schema validation
        rapidjson = pkgs.rapidjson.overrideAttrs (old: {
          cmakeFlags = (old.cmakeFlags or []) ++ [
            "-DRAPIDJSON_SCHEMA_USE_STDREGEX=ON"
            "-DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=OFF"
          ];
        });

        hermes = pkgs.stdenv.mkDerivation {
          pname = "hermes";
          version = "0.0.5";
          src = ./.;

          nativeBuildInputs = [ pkgs.cmake ];

          # Use boost186 to match libnghttp2_asio's boost version,
          # avoiding ABI/header incompatibilities
          buildInputs = [
            libnghttp2_asio
            otel-cpp
            pkgs.boost186
            pkgs.curl
            pkgs.gtest
            pkgs.openssl
            pkgs.protobuf
            rapidjson
          ];

          cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];

          installPhase = ''
            mkdir -p $out/bin
            cp src/hermes $out/bin/
          '';

          meta = with pkgs.lib; {
            description = "HTTP/2 traffic generator";
            homepage = "https://github.com/jgomezselles/hermes";
            license = licenses.mit;
            platforms = platforms.linux;
          };
        };
        container = pkgs.dockerTools.buildImage {
          name = "hermes";
          tag = "latest";
          copyToRoot = pkgs.buildEnv {
            name = "image-root";
            paths = [ hermes ];
            pathsToLink = [ "/bin" ];
          };
          config = {
            Entrypoint = [ "${hermes}/bin/hermes" ];
            Env = [ "SSL_CERT_FILE=${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt" ];
          };
        };
      in {
        packages.default = hermes;
        packages.hermes = hermes;
        packages.container = container;

        apps.default = flake-utils.lib.mkApp {
          drv = hermes;
          exePath = "/bin/hermes";
        };

        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.cmake
            pkgs.gtest
            libnghttp2_asio
            otel-cpp
            pkgs.boost186
            pkgs.curl
            pkgs.openssl
            pkgs.protobuf
            rapidjson
          ];
        };
      }
    ) // {
      # Overlay so consumers can add hermes to their nixpkgs: `nixpkgs.overlays = [ hermes.overlays.default ]` -> `pkgs.hermes`
      overlays.default = final: prev: {
        hermes = prev.stdenv.mkDerivation {
          pname = "hermes";
          version = "0.0.5";
          src = ./.;
          nativeBuildInputs = [ prev.cmake ];
          buildInputs = [
            prev.libnghttp2_asio
            (prev.opentelemetry-cpp.override { enableHttp = true; })
            prev.boost186
            prev.curl
            prev.gtest
            prev.openssl
            prev.protobuf
            (prev.rapidjson.overrideAttrs (old: {
              cmakeFlags = (old.cmakeFlags or []) ++ [
                "-DRAPIDJSON_SCHEMA_USE_STDREGEX=ON"
                "-DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=OFF"
              ];
            }))
          ];
          cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];
          installPhase = ''
            mkdir -p $out/bin
            cp src/hermes $out/bin/
          '';
          meta = with prev.lib; {
            description = "HTTP/2 traffic generator";
            homepage = "https://github.com/jgomezselles/hermes";
            license = licenses.mit;
            platforms = platforms.linux;
          };
        };
      };
    };
}

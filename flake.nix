{
  description = "Operon development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.05";

    utils.url = "github:numtide/flake-utils";

    ml-pkgs.url = "github:nixvital/ml-pkgs";
    ml-pkgs.inputs.nixpkgs.follows = "nixpkgs";
    ml-pkgs.inputs.utils.follows = "utils";    
  };

  outputs = { self, utils, nixpkgs, ml-pkgs }: {
    overlays = {
      deps = final: prev: let stdenv_ = final.clang16Stdenv; in  {
        pratt-parser = final.callPackage ./nix/pkgs/pratt-parser { stdenv = stdenv_; };
        vstat = final.callPackage ./nix/pkgs/vstat { stdenv = stdenv_; };
        lbfgs = final.callPackage ./nix/pkgs/lbfgs { stdenv = stdenv_; };
      };
      
      dev = nixpkgs.lib.composeManyExtensions [
        ml-pkgs.overlays.cc-batteries
        self.overlays.deps
      ];
    };    
  } // utils.lib.eachSystem [ "x86_64-linux" ] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ self.overlays.dev ];
        };

        stdenv_ = pkgs.overrideCC pkgs.llvmPackages_16.stdenv (
          pkgs.clang_16.override { gccForLibs = pkgs.gcc13.cc; }
        );

        operon = stdenv_.mkDerivation {
          name = "operon";
          src = self;

          cmakeFlags = [
            "-DBUILD_CLI_PROGRAMS=ON"
            "-DBUILD_SHARED_LIBS=${if pkgs.stdenv.hostPlatform.isStatic then "OFF" else "ON"}"
            "-DBUILD_TESTING=OFF"
            "-DCMAKE_BUILD_TYPE=Release"
            "-DUSE_OPENLIBM=ON"
            "-DUSE_SINGLE_PRECISION=ON"
          ];

          nativeBuildInputs = with pkgs; [ cmake git ];

          buildInputs = (with pkgs; [
            aria-csv-parser
            ceres-solver
            cpp-sort
            cxxopts
            doctest
            eigen
            eve
            fast-float
            fmt
            icu
            jemalloc
            openlibm
            pkg-config
            pratt-parser
            scnlib
            taskflow
            unordered-dense
            vstat
            lbfgs
            outcome
            quickcpplib
            status-code
            xxHash
          ]);
        };

      in rec {
        packages = {
          default = operon.overrideAttrs(old: {
            cmakeFlags = old.cmakeFlags ++ [
              "-DCMAKE_CXX_FLAGS=${
                if pkgs.stdenv.hostPlatform.isx86_64 then "-march=x86-64-v3" else ""
              }"
            ];
          });

          operon-generic = operon.overrideAttrs(old: {
            cmakeFlags = old.cmakeFlags ++ [
              "-DCMAKE_CXX_FLAGS=${
                if pkgs.stdenv.hostPlatform.isx86_64 then "-march=x86-64" else ""
              }"
            ];
          });
        };

        apps.operon-gp = {
          type = "app";
          program = "${packages.default}/bin/operon_gp";
        };

        apps.operon-nsgp = {
          type = "app";
          program = "${packages.default}/bin/operon_nsgp";
        };

        apps.parse-model = {
          type = "app";
          program = "${packages.default}/bin/operon_parse_model";
        };

        devShells.default = stdenv_.mkDerivation {
          name = "operon";

          nativeBuildInputs = operon.nativeBuildInputs ++ (with pkgs; [
            clang-tools_16
            cppcheck
            include-what-you-use
            cmake-language-server
          ]);

          buildInputs = operon.buildInputs ++ (with pkgs; [
            gdb
            graphviz
            hyperfine
            linuxPackages_latest.perf
            seer
            valgrind
            hotspot
            (mold.override { stdenv = stdenv_; })
          ]);

          shellHook = ''
            export LD_LIBRARY_PATH=$CMAKE_LIBRARY_PATH
            alias bb="cmake --build build -j"
          '';
        };
      });
}

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
      deps = final: prev: {
        pratt-parser = final.callPackage ./nix/pkgs/pratt-parser {};
        vstat = final.callPackage ./nix/pkgs/vstat {};
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
        overlays = [
          self.overlays.dev
        ];
      };
      
      operon = pkgs.stdenv.mkDerivation {
        name = "operon";
        src = self;

        cmakeFlags = [
          "-DBUILD_CLI_PROGRAMS=ON"
          "-DBUILD_SHARED_LIBS=${if pkgs.hostPlatform.isStatic then "OFF" else "ON"}"
          "-DBUILD_TESTING=OFF"
          "-DCMAKE_BUILD_TYPE=Release"
          "-DUSE_OPENLIBM=ON"
          "-DUSE_SINGLE_PRECISION=ON"
          "-DCMAKE_CXX_FLAGS=${if pkgs.hostPlatform.isx86_64 then "-march=x86-64-v3" else ""}"
        ];

        nativeBuildInputs = with pkgs; [ cmake ];

        buildInputs = (with pkgs; [
          aria-csv-parser
          cpp-sort
          cxxopts
          doctest
          eigen
          fast-float
          fmt
          git
          mold
          openlibm
          pkg-config
          pratt-parser
          scnlib
          span-lite
          taskflow
          unordered-dense
          vectorclass
          vstat
          xxhash-cpp
          spdlog
        ]);
      };

    in rec {
      packages.default = operon;

      devShells.default = pkgs.clang16Stdenv.mkDerivation {
        name = "operon-env";
        hardeningDisable = [ "all" ];
        impureUseNativeOptimizations = true;
        nativeBuildInputs = operon.nativeBuildInputs ++ (with pkgs; [
          bear
          clang-tools
          cppcheck
          include-what-you-use
        ]);

        buildInputs = operon.buildInputs ++ (with pkgs; [
          gdb
          hotspot
          valgrind
          jemalloc
          linuxPackages.perf
          graphviz
          seer
          hyperfine
        ]);
      };
    });
}

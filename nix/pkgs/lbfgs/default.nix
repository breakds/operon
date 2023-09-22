{ lib
, stdenv
, fetchFromGitHub
, cmake
, eigen
, outcome
, quickcpplib
, status-code }:

stdenv.mkDerivation {
  name = "lbfgs";
  version = "2023.08.09";


  src = fetchFromGitHub {
    owner = "foolnotion";
    repo = "lbfgs";
    rev = "0ac2cb5b8ffea5e3e71f264d8e2d37d585449512";
    hash = "sha256-3mlSjAoZceGohMl81Q5kdR1aQQBPdj8IwxBV0xLaL48=";
  };

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_CXX_FLAGS=-march=x86-64-v3"
  ];

  nativeBuildInputs = [ cmake ];

  buildInputs = [
    eigen
    outcome
    quickcpplib
    status-code
  ];
}

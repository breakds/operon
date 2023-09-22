{ lib, stdenv, fetchFromGitHub, cmake, doctest, fast-float }:

stdenv.mkDerivation rec {
  pname = "pratt-parser";
  version = "0.1.0";

  src = fetchFromGitHub {
    owner = "foolnotion";
    repo = "pratt-parser-calculator";
    rev = "025ba103339bb69e3b719b62f3457d5cbb9644e6";
    hash = "sha256-XJ05EVJJwXoYip/YtmwLmIOlaDSp7GiH53rJXE8tTlg=";
  };

  nativeBuildInputs = [ cmake ];

  buildInputs = [
    doctest
    fast-float
  ];

  meta = with lib; {
    description = "Very simple operator precedence parser following the well-known Pratt algorithm.";
    homepage = "https://github.com/foolnotion/pratt-parser-calculator";
    license = licenses.mit;
    platforms = platforms.all;
  };
}

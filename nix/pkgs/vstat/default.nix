{ lib, stdenv, fetchFromGitHub, cmake, eve }:

stdenv.mkDerivation rec {
  pname = "vstat";
  version = "1.0.0";

  src = fetchFromGitHub {
    owner = "heal-research";
    repo = "vstat";
    rev = "4ed22ae344c6a2a6e4522ad8b2c40070dd760600";
    hash = "sha256-xcQlOU6YLxykNsWnfbobrV0YmT0I3e0itRNrwxkW3jw=";
  };

  nativeBuildInputs = [ cmake ];

  buildInputs = [ eve ];

  meta = with lib; {
    description = ''
      SIMD-enabled descriptive statistics (mean, variance, covariance, correlation)
    '';
    homepage = "https://github.com/heal-research/vstat";
    license = licenses.mit;
    platforms = platforms.all;
  };
}

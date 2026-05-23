# UVM source derivation (IEEE 1800.2-2020, Apache-2.0).
# https://github.com/accellera-official/uvm-core
#
# The 1800.2-2020 release is a superset of 1800.2-2017 — safe to use as an
# include path when linting projects written against the 2017 standard.
#
# To update the hash after changing rev:
#   nix-prefetch-github accellera-official uvm-core --rev <new-rev>
{ pkgs }:
pkgs.fetchFromGitHub {
  owner  = "accellera-official";
  repo   = "uvm-core";
  rev    = "2020.3.1";
  sha256 = "sha256-o8YFzGFfcP+AcSjpoV6pOpiTQEbtDF3TKM6p7oDGJrU=";
}

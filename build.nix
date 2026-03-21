{ pkgs ? import <nixpkgs> {} }:
let
  shared = import ./nix/shared.nix {
    inherit pkgs;
  };
in
pkgs.stdenv.mkDerivation {
  pname = "verihogg-lint";
  version = "0.1.0";

  src = ./.;

  nativeBuildInputs = shared.nativeBuildInputs;

  doCheck = true;

  buildInputs = shared.buildInputs;

  meta = with pkgs.lib; {
    description = "SystemVerilog linter powered by Surelog";
    license = licenses.mit;
    platforms = platforms.linux;
  };
}

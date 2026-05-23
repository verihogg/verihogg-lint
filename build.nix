{ pkgs ? import <nixpkgs> {} }:
let
  shared  = import ./nix/shared.nix { inherit pkgs; };
  uvm-src = import ./nix/uvm.nix    { inherit pkgs; };
in
pkgs.stdenv.mkDerivation {
  pname = "verihogg-lint";
  version = "0.2.0";

  src = ./.;

  nativeBuildInputs = shared.nativeBuildInputs;

  doCheck = true;
  enableParallelChecking = false;

  buildInputs = shared.buildInputs;

  cmakeFlags = [
    "-DUVM_SRC_DIR=${uvm-src}/src"
  ];

  meta = with pkgs.lib; {
    description = "SystemVerilog linter powered by Surelog";
    license = licenses.mit;
    platforms = platforms.linux;
  };

  preConfigure=''
    rm -rf build
  '';
}

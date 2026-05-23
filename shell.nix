{ pkgs ? import <nixpkgs> {} }:
let
  shared  = import ./nix/shared.nix { inherit pkgs; };
  uvm-src = import ./nix/uvm.nix    { inherit pkgs; };
in
pkgs.mkShell {
  packages =
    shared.nativeBuildInputs
    ++ shared.buildInputs
    ++ shared.shellOnlyPackages;

  shellHook = ''
    export SURELOG_ROOT=${pkgs.surelog}
    export VERIHOGG_UVM_PATH=${uvm-src}/src
  '';
}

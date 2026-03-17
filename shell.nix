{ pkgs ? import <nixpkgs> {} }:
let
  shared = import ./nix/shared.nix {
    inherit pkgs;
  };
in
pkgs.mkShell {
  packages =
    shared.nativeBuildInputs
    ++ shared.buildInputs
    ++ shared.shellOnlyPackages;
}

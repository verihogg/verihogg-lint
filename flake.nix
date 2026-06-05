{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    systems.url = "github:nix-systems/default";
  };

  outputs = { self, nixpkgs, systems, ... }:
  let
    eachSystem = fn:
      nixpkgs.lib.genAttrs
      (import systems)
      (system: fn system nixpkgs.legacyPackages.${system});
  in {
    packages = eachSystem (system: pkgs: rec {
      verihogg-lint = pkgs.callPackage ./default.nix { inherit pkgs; };
      default = verihogg-lint;
    });

    devShells = eachSystem (system: pkgs: {
      default = import ./shell.nix { inherit pkgs; };
    });
  };
}

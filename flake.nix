{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    nixpkgs-patcher.url = "github:gepbird/nixpkgs-patcher";
    flake-parts.url = "github:hercules-ci/flake-parts";
    systems.url = "github:nix-systems/default";
    flake-compat = {
      url = "github:NixOS/flake-compat";
      flake = false;
    };
    uvm = {
      url = "github:accellera-official/uvm-core?ref=2020.3.1";
      flake = false;
    };
    nixpkgs-patch-clang-tools-bash = {
      url = "https://github.com/NixOS/nixpkgs/pull/563394.diff";
      flake = false;
    };
  };

  outputs =
    inputs@{
      self,
      nixpkgs,
      nixpkgs-patcher,
      flake-parts,
      systems,
      uvm,
      ...
    }:
    flake-parts.lib.mkFlake { inputs = inputs // { }; } {
      systems = import systems;
      perSystem =
        {
          self',
          pkgs,
          system,
          ...
        }:
        let
          inherit (pkgs) lib;
        in
        {
          packages.default = self'.packages.verihogg-lint;
          packages.verihogg-lint = pkgs.stdenv.mkDerivation {
            pname = "verihogg-lint";
            version = "0.3.0";
            src = ./.;

            meta = {
              description = "SystemVerilog linter powered by Surelog";
              license = lib.licenses.mit;
              platforms = lib.platforms.linux;
            };

            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
              gtest
              microsoft-gsl
            ];

            buildInputs =
              with pkgs;
              [
                surelog
                yaml-cpp
              ]
              ++ surelog.buildInputs;

            cmakeFlags = [
              "-DUVM_SRC_DIR=${uvm}/src"
            ];

            enableParallelChecking = false;
          };

          devShells.default =
            let
              nixpkgs-patched = nixpkgs-patcher.lib.patchNixpkgs { inherit inputs system; };
              pkgs = import nixpkgs-patched { inherit system; };
            in
            pkgs.mkShell {
              buildInputs =
                self'.packages.default.buildInputs
                ++ self'.packages.default.nativeBuildInputs
                ++ [ pkgs.llvmPackages_22.clang-tools ];
            };
        };
    };
}

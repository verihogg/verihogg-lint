{ pkgs ? import <nixpkgs> {} }:
let
  surelog = pkgs.surelog.overrideAttrs (_: {
    src = builtins.fetchGit {
      url = "https://github.com/verihogg/Surelog.git";
      rev = "d3498b89eeaaf792af2ff5bfbe21ea2fc32d9eda";
      shallow = true;
    };
    version = "d3498b89";
  }); 
in {
  inherit surelog;

  buildInputs = with pkgs; [
    surelog
  ] ++ surelog.buildInputs;

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
  ];

  shellOnlyPackages = with pkgs; [
    llvmPackages_19.clang-tools
  ];
}

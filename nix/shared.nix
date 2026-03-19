{ pkgs ? import <nixpkgs> {} }:
let
  surelog = pkgs.surelog.overrideAttrs (_: {
    src = builtins.fetchGit {
      url = "https://github.com/chipsalliance/Surelog.git";
      rev = "35ec4b3fa6b98464fec12b5da3530e7bd7b5c4c7";
      shallow = true;
    };
    version = "35ec4b3f";
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

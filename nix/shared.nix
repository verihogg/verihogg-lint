{ pkgs ? import <nixpkgs> {} }: 
{
  buildInputs = with pkgs; [
    surelog
    yaml-cpp
  ] ++ surelog.buildInputs;

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
    gtest
  ];

  shellOnlyPackages = with pkgs; [
    llvmPackages_21.clang-tools
  ];
}

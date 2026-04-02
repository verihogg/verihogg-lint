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
    microsoft-gsl
  ];

  shellOnlyPackages = with pkgs; [
    llvmPackages_22.clang-tools
  ];

}
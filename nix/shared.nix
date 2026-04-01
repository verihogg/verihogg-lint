{ pkgs ? import <nixpkgs> {} }: 
{
  buildInputs = with pkgs; [
    surelog
  ] ++ surelog.buildInputs;

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
  ];

  shellOnlyPackages = with pkgs; [
    llvmPackages_21.clang-tools
  ];
}

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
    llvmPackages_19.clang-tools
  ];
}

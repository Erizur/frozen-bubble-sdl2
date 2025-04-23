{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
   buildInputs = [
     pkgs.clang
     pkgs.clang-tools
     pkgs.lldb
     pkgs.ninja 
     pkgs.cmake

     pkgs.SDL2
     pkgs.SDL2_mixer
     pkgs.SDL2_ttf
     pkgs.SDL2_image
     pkgs.iniparser
     pkgs.pkg-config
     pkgs.glib

     # Original game for development purposes
     pkgs.frozen-bubble
   ];
  # Use clang's stdenv
  stdenv = pkgs.llvmPackages_latest.stdenv;
}

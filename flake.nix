{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/24.11";
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
  };
  outputs = { nixpkgs, flake-utils, ... }: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };
      frozen-bubble-sdl2 = (with pkgs; stdenv.mkDerivation {
          pname = "frozen-bubble-sdl2";
          version = "0.1";
          src = ./.;
          nativeBuildInputs = [
            clang
	    clang-tools
            cmake
	    ninja
	    SDL2
	    SDL2.dev
 	    SDL2_ttf
	    SDL2_image
	    SDL2_mixer
	    iniparser
	    pkg-config
	    glib
          ];
          bu0ildPhase = "cd build && ninja -j $NIX_BUILD_CORES";
          installPhase = ''
            mkdir -p $out/bin
            mv ./frozen-bubble $out/bin
          '';
        }
      );
    in rec {
      defaultApp = flake-utils.lib.mkApp {
        drv = defaultPackage;
      };
      defaultPackage = frozen-bubble-sdl2;
      devShell = pkgs.mkShell {
        buildInputs = [
          frozen-bubble-sdl2
        ];
      };
    }
  );
}

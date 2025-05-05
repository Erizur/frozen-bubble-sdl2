{pkgs ? import <nixpkgs> {}}:
pkgs.clangStdenv.mkDerivation {
  pname = "frozen-bubble-sdl2";
  version = "0.1.0";

  src = ./.;

  nativeBuildInputs = with pkgs; [cmake];
  buildInputs = with pkgs; [clang ninja SDL2 SDL2.dev SDL2_ttf SDL2_image SDL2_mixer libpng libjpeg libtiff libwebp iniparser glib pkg-config];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  buildPhase = ''
    cmake . -B build $cmakeFlags
    cmake --build build
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp build/frozen-bubble $out/bin/frozen-bubble-sdl2
  '';

  meta = with pkgs.lib; {
    description = "SDL2 C++ Port of Frozen-Bubble 2";
    homepage = "https://github.com/Erizur/frozen-bubble-sdl2";
    license = licenses.gpl2;
  };
}

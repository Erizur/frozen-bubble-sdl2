{
  lib,
  clangStdenv,
  cmake,
  clang,
  ninja,
  SDL2,
  SDL2_ttf,
  SDL2_image,
  SDL2_mixer,
  libpng,
  libjpeg,
  libtiff,
  libwebp,
  iniparser,
  glib,
  pkg-config,
  ...
}:
clangStdenv.mkDerivation {
  pname = "frozen-bubble-sdl2";
  version = "0.1.0";

  src = lib.cleanSource ./.;

  nativeBuildInputs = [ cmake ];

  buildInputs = [
    clang
    ninja
    SDL2
    SDL2.dev
    SDL2_ttf
    SDL2_image
    SDL2_mixer
    libpng
    libjpeg
    libtiff
    libwebp
    iniparser
    glib
    pkg-config
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  buildPhase = ''
    cmake . -B build -DASSET_PATH="$out/share" $cmakeFlags
    cmake --build build
  '';

  installPhase = ''
    mkdir -p "$out/bin"
    cp -r ../share "$out/share"
    cp build/frozen-bubble-sdl2 "$out/bin"
  '';

  meta = with lib; {
    description = "SDL2 C++ Port of Frozen-Bubble 2";
    homepage = "https://github.com/Erizur/frozen-bubble-sdl2";
    license = licenses.gpl2;
  };
}

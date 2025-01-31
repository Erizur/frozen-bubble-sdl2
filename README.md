# Frozen-Bubble: SDL2
<p style="text-align: center"><img src="https://github.com/user-attachments/assets/c68db5c9-7e72-4d19-8e98-c598a3f5e54e"></p>

An ongoing port of the original Frozen-Bubble 2 into a newer C++ codebase using SDL2. I am planning on reimplementing most features from the original Frozen-Bubble 2 into the codebase, but some of them might be out of my knowledge. Feel free to contribute if you want!

Checklist: https://github.com/Erizur/frozen-bubble-sdl2/issues/1

# Building
You need to obtain the following libraries: 
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf
- [iniparser](https://github.com/ndevilla/iniparser)

Make sure these are detected by CMake. After that, you're ready to compile!
You will be able to compile the server-related code if you're trying to compile for macOS and Linux.
For the moment, the DATA_DIR path is fixed to the path of the repository, unless you specify your own path.

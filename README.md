# Frozen-Bubble: SDL2
An ongoing port of the original Frozen-Bubble 2 into a newer C++ codebase using SDL2. I am plan on reimplementing most features from the original Frozen-Bubble 2 into the codebase, but some of them might be out of my knowledge, but feel free to contribute if you want!

**Currently implemented:**
- [X] Title Screen
    - [ ] Fully working menu buttons
    - [X] Credits banner
    - [X] Graphics adjust button
- [X] Game Settings
    - [ ] Default settings
    - [ ] Read highscores
    - [X] Read data from settings.ini
- [X] Audio Mixer
    - [ ] 1P and 2P in-game songs
    - [X] Main menu theme
    - [X] Sound effects loader

**To do (important)**
- [ ] Game modes
    - [ ] 1P (Default levelset & random)
    - [ ] 2P
- [ ] Highscores
    - [ ] Save/load highscores
- [ ] Level editor
    - [ ] Menu & instructions
    - [ ] Save/load levels

**To do (low priority)**
- [ ] LAN multiplayer / Netplay
    - [ ] Fix server code
    - [ ] Reimplement multiplayer logic
- [ ] Packaging / Distribution
    - [ ] Easier deployment

# Building
You need to install SDL2, SDL2_image, SDL2_mixer, iniparser-static and glib in order to install.
After that, just compile with CMake. Detailed instructions will be done soon.
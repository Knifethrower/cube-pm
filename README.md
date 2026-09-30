# Cube for PortMaster

This is an **altered version** of the Cube engine source code, 2005_08_29 release, by Wouter van
Oortmerssen (http://cubeengine.com/, https://sourceforge.net/projects/cube/). See `readme.txt` for
the original license (ZLIB with an additional clause) and documentation.

Changes for the PortMaster port (aarch64 Linux handhelds):

- SDL 1.2 → SDL2: window and GL context, relative mouse mode, text input for the command line,
  mouse wheel as buttons 4/5 as before, SDL2's key codes translated to the SDL 1.2 numbers that
  `data/keymap.cfg` and saved configs use (`sdl1key` in `main.cpp`), window brightness for
  `gamma`, clipboard paste through SDL, resolver threads detached instead of killed.
- Textures load with stb_image (`stb_image.h`, public domain / MIT) instead of SDL_image.
- Fills the screen on every aspect ratio: the field of view keeps the 4:3 vertical view on wider
  screens and the 4:3 horizontal view on narrower ones (`hfov()`), the culling uses the angle
  actually shown, and the HUD's virtual screen follows the screen's shape with its bottom and
  right elements anchored to the edges.
- `src/Makefile`: client only, ENet built in, GLU linked statically.

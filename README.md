# GPWE - (G)eneral (P)urpose (W)orld (E)ngine

## Dependencies

- SDL2
- Freetype
- FreeImage

### Ubuntu

```bash
sudo apt install libsdl2-dev libfreetype-dev libfreeimage-dev
```

## Building

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j$(nproc)
```

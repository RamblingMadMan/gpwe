<h1 align="center">GPWE<br />General Purpose World Engine</h1>

<p><br /></p>

GPWE is an engine designed primarily for creating games and 3D environments, but can be used for worlds of any kind.

<p><br /></p>

<h2 align="center">Features<br /></h2>

<p><br /></p>

<div align="center">

| Feature | Status |
|:--------|:------:|
| HDR deferred renderer | ✅ |
| Real-time ray tracing | ❌ |
| Bullet physics integration | ❌ |

</div>
  
<p><br /></p>

<h2 align="center">Dependencies</h2>
  
<p><br /></p>

<div align="center">

| Dependency | Included |
|:-----------|:--------:|
| SDL2       | ❌ |
| Freetype   | ❌ |
| FreeImage  | ❌ |
| Assimp     | ✅ |
| glm        | ✅ |
| glbinding  | ✅ |

</div>

<p><br /></p>

### Installing system dependencies:

<p><br /></p>

#### Ubuntu

```bash
sudo apt install libsdl2-dev libfreetype-dev libfreeimage-dev
```
<p><br /></p>

<h2 align="center">Building</h2>

<p><br /></p>

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j$(nproc)
```

<p><br /></p>

<h2 align="center">Screenshots</h2>

<p><br /></p>

There isn't much to show off yet, so here's a cube with fake lighting:

<p align="center">
<img src="res/gpwe1.png" width="512" alt="Screenshot of a cube" />
</p>

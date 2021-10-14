<h1 align="center">GPWE<br />General Purpose World Engine</h1>

<p><br /></p>

<p align="center">
GPWE is an engine designed primarily for creating games and 3D environments, but can be used for worlds of any kind.
</p>
  
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
| libmagic   | ❌ |
| Freetype   | ❌ |
| FreeImage  | ❌ |
| Assimp     | ✅ |
| glm        | ✅ |
| glbinding  | ✅ |

</div>

<p><br /></p>

<h3 align="center">Installing system dependencies</h3>

<p><br /></p>

<h4 align="center">Ubuntu</h4>

```bash
sudo apt install libmagic-dev libsdl2-dev libfreetype-dev libfreeimage-dev libbullet-dev
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

<p align="center">
There isn't much to show off yet, so here's a cube with fake lighting.
</p>
  
<div align="center">
<img align="center" src="res/gpwe1.png" width="512" alt="Screenshot of a cube" />
</div>

# Getting Started

## Getting GPWE

```bash
git clone --recursive --depth 1 https://github.com/RamblingMadMan/gpwe.git
```

## Building GPWE

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j $(nproc)
```

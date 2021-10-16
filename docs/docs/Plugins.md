# Plugins

## Creating a plugin

First make sure you have a basic CMake project set up.

Then we create a new target using the function `gpwe_add_plugin`.

Here's an example of a renderer plugin:

```cmake
set(
	RENDERER_NULL_SOURCES
	RendererNull.hpp
	RendererNull.cpp
)

gpwe_add_plugin(renderer-null ${RENDERER_NULL_SOURCES})
```

## Embedding resources in a plugin

Resources can be embedded in the final binary (shared library) of a plugin using `plugin_add_resources`.

Once a file with the relative path `<file>` is added to a plugin it is made available through the header `<file>.hpp`.

This header exposes the following functions:

```c++
const char *gpwe::embed::<file_name>_data() noexcept;
std::size_t gpwe::embed::<file_name>_data() noexcept;
std::string_view gpwe::embed::<file_name>_str() noexcept;
```

`<filename>` is the same as `<file>` but with all spaces, fullstops and slashes replaced with `_`.

Rather than go on, here is the previous example with embedded shaders:

```CMake
set(
	RENDERER_GL_SOURCES
	RendererGL.hpp
	RendererGL.cpp
)

set(
	RENDERER_GL_SHADERS
	shaders/simple.vert
	shaders/simple.frag
)

gpwe_add_plugin(renderer-gl ${RENDERER_GL_SOURCES})

plugin_add_resources(renderer-gl ${RENDERER_GL_SHADERS})
```

Then we would access the shaders from `RendererGL.cpp` like so:

```c++
#include "shaders/simple.vert.hpp"
#include "shaders/simple.frag.hpp"

void RendererGL::doRendererThings(){
	// ...
	
	auto vertSrc = gpwe::embed::shaders_vert_str();
	auto fragSrc = gpwe::embed::shaders_frag_str();

	// ...
}
```

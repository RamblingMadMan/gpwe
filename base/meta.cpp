#include "gpwe/util/meta.hpp"
#include "gpwe/sys.hpp"

using namespace gpwe;

#define MAP_OBJECT_MANAGER(obj, ns)\
ns::Manager *meta::detail::ObjectBaseManagerHelper<obj>::get(){ return ns::manager(); }

#define MAP_RENDER_OBJECT(obj)\
MAP_OBJECT_MANAGER(obj, render)

#define MAP_INPUT_OBJECT(obj)\
MAP_OBJECT_MANAGER(obj, input)

#define MAP_PHYSICS_OBJECT(obj)\
MAP_OBJECT_MANAGER(obj, physics)

#define MAP_WORLD_OBJECT(obj)\
MAP_OBJECT_MANAGER(obj, world)

#define MAP_UI_OBJECT(obj)\
MAP_OBJECT_MANAGER(obj, ui)

MAP_INPUT_OBJECT(InputKeyboard)
MAP_INPUT_OBJECT(InputMouse)
MAP_INPUT_OBJECT(InputGamepad)

MAP_RENDER_OBJECT(RenderFramebuffer)
MAP_RENDER_OBJECT(RenderTexture)
MAP_RENDER_OBJECT(RenderPipeline)
MAP_RENDER_OBJECT(RenderProgram)
MAP_RENDER_OBJECT(RenderGroup)
MAP_RENDER_OBJECT(RenderInstance)
MAP_RENDER_OBJECT(RenderInstanceData)

MAP_PHYSICS_OBJECT(PhysicsWorld)
MAP_PHYSICS_OBJECT(PhysicsBody)
MAP_PHYSICS_OBJECT(PhysicsBodyShape)

MAP_WORLD_OBJECT(WorldBlock)
MAP_WORLD_OBJECT(WorldEntity)

MAP_UI_OBJECT(UiSolidColor)
MAP_UI_OBJECT(UiLayout)

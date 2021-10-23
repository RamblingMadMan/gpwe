#ifndef GPWE_TYPES_HPP
#define GPWE_TYPES_HPP 1

#include <cstdint>

namespace gpwe::log{
	class Manager;
}

namespace gpwe::sys{
	class Manager;
}

namespace gpwe::resource{
	class Manager;
	class Plugin;
}

namespace gpwe::input{
	class Manager;
	class System;
	class Keyboard;
	class Mouse;
	class Gamepad;
}

namespace gpwe::render{
	class Manager;
	class Framebuffer;
	class Texture;
	class Pipeline;
	class Program;
	class Group;
	class Instance;
	class InstanceData;
}

namespace gpwe::physics{
	class Manager;
	class World;
	class Body;
	class BodyShape;
}

namespace gpwe::app{
	class Manager;
}

namespace gpwe::world{
	class Manager;
	class Block;
	class Entity;
}

namespace gpwe::ui{
	class Manager;
	class WidgetBase;
	class SolidColor;
	class Layout;
}

namespace gpwe{
	enum class Axis{
		x, y, z,
		count
	};

	using Nat8 = std::uint8_t;
	using Nat16 = std::uint16_t;
	using Nat32 = std::uint32_t;
	using Nat64 = std::uint64_t;

	using Int8 = std::int8_t;
	using Int16 = std::int16_t;
	using Int32 = std::int32_t;
	using Int64 = std::int64_t;

	using Float32 = float;
	using Float64 = double;

	class ObjectBase;
	class PropertyBase;

	class Camera;
	using LogManager = log::Manager;
	using ResourceManager = resource::Manager;
	using SysManager = sys::Manager;
	using AppManager = app::Manager;

	using InputManager = input::Manager;
	using InputSystem = input::System;
	using InputKeyboard = input::Keyboard;
	using InputMouse = input::Mouse;
	using InputGamepad = input::Gamepad;

	using RenderManager = render::Manager;
	using RenderFramebuffer = render::Framebuffer;
	using RenderTexture = render::Texture;
	using RenderPipeline = render::Pipeline;
	using RenderProgram = render::Program;
	using RenderGroup = render::Group;
	using RenderInstance = render::Instance;
	using RenderInstanceData = render::InstanceData;

	using PhysicsManager = physics::Manager;
	using PhysicsWorld = physics::World;
	using PhysicsBody = physics::Body;
	using PhysicsBodyShape = physics::BodyShape;

	using WorldManager = world::Manager;
	using WorldBlock = world::Block;
	using WorldEntity = world::Entity;

	using UiManager = ui::Manager;
	using UiWidgetBase = ui::WidgetBase;
	using UiSolidColor = ui::SolidColor;
	using UiLayout = ui::Layout;
}

#endif // !GPWE_TYPES_HPP

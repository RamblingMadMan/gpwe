#ifndef GPWE_RENDERER_GL43_SHADERS_HPP
#define GPWE_RENDERER_GL43_SHADERS_HPP 1

#include <string_view>

namespace gpwe{
	constexpr std::string_view vertFullbright =
			"#version 430 core\n"

			"out gl_PerVertex\n"
			"{\n"
			"	vec4 gl_Position;\n"
			"	float gl_PointSize;\n"
			"	float gl_ClipDistance[];\n"
			"};\n"

			"layout(location = 0) in vec3 vert;\n"
			"layout(location = 1) in vec3 norm;\n"
			"layout(location = 2) in vec2 uv;\n"

			"uniform mat4 viewProj;\n"

			"out vec3 norm_v;\n"
			"out vec2 uv_v;\n"

			"void main(){\n"
			"	norm_v = norm;\n"
			"	uv_v = uv;\n"
			"	gl_Position = viewProj * vec4(vert, 1.0);\n"
			"}\n"
		;

	constexpr std::string_view fragFullbright =
			"#version 430 core\n"

			"layout(location = 0) out vec4 outAlbedo;\n"
			"layout(location = 1) out vec3 outNormal;\n"

			"in vec3 norm_v;\n"
			"in vec2 uv_v;\n"

			"void main(){\n"
			"	float illum = max(norm_v.y, 0.1) + max(abs(norm_v.z * 0.5), 0.1);"
			"	outAlbedo = vec4(vec3(illum), 1.0);\n"
			"	outNormal = norm_v;\n"
			"}\n"
		;

	constexpr std::string_view vertPost =
			"#version 430 core\n"

			"out gl_PerVertex\n"
			"{\n"
			"	vec4 gl_Position;\n"
			"	float gl_PointSize;\n"
			"	float gl_ClipDistance[];\n"
			"};\n"

			"layout(location = 0) in vec3 vert;\n"
			"layout(location = 2) in vec2 uv;\n"

			"out vec2 uv_v;\n"

			"void main(){\n"
			"	uv_v = uv;\n"
			"	gl_Position = vec4(vert, 1.0);\n"
			"}\n"
		;

	constexpr std::string_view fragPost =
			"#version 430 core\n"

			"layout(location = 0) out vec4 outScreen;\n"

			"uniform sampler2D texAlbedo;\n"
			"uniform sampler2D texNormal;\n"

			"in vec2 uv_v;\n"

			"void main(){\n"
			"	outScreen = vec4(texture(texAlbedo, uv_v).rgb, 1.0);"
			"}"
		;
}

#endif // !GPWE_RENDERER_GL43_SHADERS_HPP

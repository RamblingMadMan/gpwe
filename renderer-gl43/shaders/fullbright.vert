#version 430 core

out gl_PerVertex{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;

uniform mat4 viewProj;

out vec3 norm_v;
out vec2 uv_v;

void main(){
	norm_v = norm;
	uv_v = uv;
    gl_Position = viewProj * vec4(vert, 1.0);
}

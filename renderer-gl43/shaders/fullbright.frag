#version 430 core

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec3 outNormal;

in vec3 norm_v;
in vec2 uv_v;

uniform vec4 colorMix;
uniform sampler2D tex;

void main(){
	outAlbedo = vec4(vec3(max(norm_v.y, 0.0)), 1.0);
	outNormal = norm_v;
}

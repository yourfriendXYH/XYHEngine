#version 310 es

#extension GL_GOOGLE_include_directive : enable

layout(set = 0, binding = 0) readonly buffer perFrame
{
	mat4 projViewMatrix;
};

layout(set = 0, binding = 1) readonly buffer perDrawcall
{
	mat4 modelMatrix;
};

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 fragColor;

void main()
{
	vec3 worldPosition = (modelMatrix * vec4(in_position, 0.0, 1.0)).xyz;
	gl_Position = projViewMatrix * vec4(worldPosition, 1.0);	// 
	fragColor = in_color;
}
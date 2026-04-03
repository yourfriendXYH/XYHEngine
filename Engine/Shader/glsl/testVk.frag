#version 310 es

#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in highp vec3 fragColor;

layout(location = 0) out highp vec4 outColor;

void main()
{
	outColor = vec4(fragColor, 1.0);
}
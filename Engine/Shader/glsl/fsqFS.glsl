#version 450

layout(location = 0) in vec4 V_Texcoord;

layout(binding = 0) uniform sampler2D U_VisualizationTexture;

layout(location = 0) out vec4 RT0;

void main()
{
	vec3 color = texture(U_VisualizationTexture, V_Texcoord.xy).rgb;
	RT0 = vec4(color, 1.0);
}
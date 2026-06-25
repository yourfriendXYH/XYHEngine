#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1)in;

layout(binding = 0)uniform GlobalConstants
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ModelMatrix;
	uvec4 Misc0;
	vec4 CameraPositionWS;
	vec4 ViewDirectionWS;
};

layout(binding = 1, std430)buffer FBVH
{
	uint m_data[];
}BVH;

layout(binding = 2, std430)buffer FCurrentIndirectWorkArgs
{
	uint m_data[];
}CurrentIndirectWorkArgs;

layout(binding = 3, std430)buffer FNextIndirectWorkArgs
{
	uint m_data[];
}NextIndirectWorkArgs;

layout(binding = 4, std430)buffer FMainAndPostNodeAndClusterBatches
{
	uint m_data[];
}MainAndPostNodeAndClusterBatches;


void main()
{
}
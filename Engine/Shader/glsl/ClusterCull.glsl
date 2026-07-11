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

layout(binding = 1, std430)buffer FNaniteMesh
{
	uint m_data[];
}NaniteMesh;

layout(binding = 2, std430)buffer FIndirectWorkArgs
{
	uint m_data[];
}IndirectWorkArgs;

layout(binding = 3, std430)buffer FVisibleClusterSoftwareHardware
{
	uvec2 m_data[];
}VisibleClusterSwHw;

layout(binding = 4, std430)buffer FMainAndPostNodeAndClusterBatches
{
	uint m_data[];
}MainAndPostNodeAndClusterBatches;


void main()
{
	uint clusterCount = IndirectWorkArgs.m_data[1];	// 获取cluster数量
	// 传递cluster内存分页所在位置
	for (uint index = 0; index < clusterCount; ++index)
	{
		VisibleClusterSwHw.m_data[index].x = MainAndPostNodeAndClusterBatches.m_data[1024 + index * 2];
		VisibleClusterSwHw.m_data[index].y = MainAndPostNodeAndClusterBatches.m_data[1024 + index * 2 + 1];
	}
}
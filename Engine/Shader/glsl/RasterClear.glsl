#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1)in;

layout(binding = 0, std430)buffer FCurrentIndirectWorkArgs
{
	uint m_data[];
}CurrentIndirectWorkArgs;

layout(binding = 1, std430)buffer FNextIndirectWorkArgs
{
	uint m_data[];
}NextIndirectWorkArgs;

layout(binding = 2, std430)buffer FVisBuffer64
{
	uint64_t m_data[];
}VisBuffer64;

void main()
{
	ivec2 texcoord = ivec2(gl_GlobalInvocationID.xy);
	if(any(greaterThanEqual(texcoord, ivec2(1280, 720))))
	{
		return;
	}
	int pixelIndex = texcoord.x + texcoord.y * 1280;

	VisBuffer64.m_data[pixelIndex] = 0xFFFFFFFF00000000ul;	// 高位为深度值，低位为ClusterIndex

	//VisBuffer64.m_data[pixelIndex] = 0x0000000000000000ul;	// 高位为深度值，低位为ClusterIndex

	if (texcoord.x == 0 && texcoord.y == 0)
	{
		// 前5个是间接绘制命令参数
		CurrentIndirectWorkArgs.m_data[0] = 384;	// 每个cluster最多128个三角形，384个索引
		CurrentIndirectWorkArgs.m_data[1] = 0;		// cluster的总数量
		CurrentIndirectWorkArgs.m_data[5] = 0;
		CurrentIndirectWorkArgs.m_data[6] = 1;

		NextIndirectWorkArgs.m_data[0] = 384;	// 每个cluster最多128个三角形，384个索引
		NextIndirectWorkArgs.m_data[1] = 0;		// cluster的总数量
		NextIndirectWorkArgs.m_data[5] = 0;
		NextIndirectWorkArgs.m_data[6] = 1;
	}
}
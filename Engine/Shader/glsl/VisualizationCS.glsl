#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1)in;

layout(binding = 0, std430)buffer FVisBuffer64
{
	uint64_t m_data[];
}VisBuffer64;

layout(binding = 0, rgba32f)uniform image2D VisualizationTexture;

// 源码位置 Hash.ush
uint MurmurMix(uint Hash)
{
	Hash ^= Hash >> 16;
	Hash *= 0x85ebca6b;
	Hash ^= Hash >> 13;
	Hash *= 0xc2b2ae35;
	Hash ^= Hash >> 16;
	return Hash;
}

// 源码位置 Visualization.ush
vec3 IntToColor(uint Index)
{
	uint Hash = MurmurMix(Index);

	vec3 Color = vec3
	(
		(Hash >>  0) & 255,
		(Hash >>  8) & 255,
		(Hash >> 16) & 255
	);

	return Color * (1.0f / 255.0f);
}

void main()
{
	ivec2 texcoord = ivec2(gl_GlobalInvocationID.xy);
	if(any(greaterThanEqual(texcoord, ivec2(1280, 720))))
	{
		return;
	}
	int pixelIndex = texcoord.y * 1280 + texcoord.x;
	uint clusterIndex = uint(VisBuffer64.m_data[pixelIndex]);	// 获取低位ClusterIndex
	vec3 Result = vec3(0.0, 0.0, 0.0);
	if (clusterIndex != 0)
	{
		clusterIndex = clusterIndex & 0xFFu - 1;
		// 源码位置 NaniteVisualize.usf
		Result = IntToColor(clusterIndex);
		Result = Result * 0.8 + 0.2;
	}

//	uint64_t pixelValue = VisBuffer64.m_data[pixelIndex];
//	float testColorValue = float(uint(pixelValue) / 255.0f);
//	imageStore(VisualizationTexture, texcoord, vec4(testColorValue, testColorValue, testColorValue, 1.0));
	imageStore(VisualizationTexture, texcoord, vec4(Result, 1.0));
}
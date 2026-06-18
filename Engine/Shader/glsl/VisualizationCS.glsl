#version 450
//#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1)in;

layout(binding = 0, std430)buffer VisBuffer64
{
	uint m_data[];
};

layout(binding = 0, rgba32f)uniform image2D VisualizationTexture;

void main()
{
	for(int x = 0; x < 1280; ++x)
	{
		for(int y = 0; y < 720; ++y)
		{
			imageStore(VisualizationTexture, ivec2(x, y), vec4(0.5, 0.5, 0.5, 1.0));
		}
	}
}
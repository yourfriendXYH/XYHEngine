#version 450
//#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1)in;

layout(binding = 0, std430)buffer FVisBuffer64
{
	uint m_data[];
}VisBuffer64;

layout(binding = 0, rgba32f)uniform image2D VisualizationTexture;

void main()
{
	ivec2 texcoord = ivec2(gl_GlobalInvocationID.xy);
	if(any(greaterThanEqual(texcoord, ivec2(1280, 720))))
	{
		return;
	}
	imageStore(VisualizationTexture, texcoord, vec4(0.0, 1.0, 0.0, 1.0));
}
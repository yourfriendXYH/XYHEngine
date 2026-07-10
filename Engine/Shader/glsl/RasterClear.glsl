#version 450

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
	uint m_data[];
}VisBuffer64;

void main()
{
	ivec2 texcoord = ivec2(gl_GlobalInvocationID.xy);
	if(any(greaterThanEqual(texcoord, ivec2(1280, 720))))
	{
		return;
	}
	int pixelIndex = texcoord.x + texcoord.y * 1280;
	VisBuffer64.m_data[pixelIndex] = 0u;

	if (texcoord.x == 0 && texcoord.y == 0)
	{
		CurrentIndirectWorkArgs.m_data[0] = 0;
		CurrentIndirectWorkArgs.m_data[1] = 1;
		NextIndirectWorkArgs.m_data[0] = 0;
		NextIndirectWorkArgs.m_data[1] = 1;
	}
}
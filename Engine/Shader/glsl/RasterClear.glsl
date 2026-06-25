#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1)in;

layout(binding = 0, std430)buffer FCurrentIndirectWorkArgs
{
	uint m_data[];
}CurrentIndirectWorkArgs;

layout(binding = 1, std430)buffer FNextIndirectWorkArgs
{
	uint m_data[];
}NextIndirectWorkArgs;

layout(binding = 2, std430)buffer VisBuffer64
{
	uint m_data[];
};

void main()
{
}
#version 450

layout(binding = 0)uniform GlobalConstants
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ModelMatrix;
	uvec4 Misc0;
	vec4 CameraPositionWS;
	vec4 ViewDirectionWS;
};

layout(binding = 1, std430)readonly buffer NaniteMesh
{
	uint mData[];
};

layout(binding = 2, std430)readonly buffer VisibleClusterSoftwareHardware
{
	uint mData[];
}VisibleClusterSwHw;

void main()
{
	uint vertexIndex = gl_VertexID;
	uint clusterIndexWithInvoke = gl_InstanceID;
	gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
};
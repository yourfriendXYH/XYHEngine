#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1)in;

layout(binding = 0)uniform GlobalConstants
{
	mat4 ProjectionMatrix;
    mat4 ProjMatrix;
	mat4 ViewMatrix;
	mat4 ViewMatrixWithoutTranslate;  // 此ViewMatrix不允许带相机偏移
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

layout(binding = 5, std430)readonly buffer FPerDrawcallBuffer
{
    mat4 m_modelMatrix;
}PerDrawcallBuffer;

struct ClusterInfo
{
    uint m_baseOffset;
    uint m_indexOffset;
    uint m_indexCount;
    vec4 m_LODBounds;
    float m_LODError;
    float m_edgeLength;
};

// 解码 自定义内存分页 的数据
ClusterInfo GetClusterInfo(uint inPageIndex,uint inClusterIndex)
{
    uint pageCount = NaniteMesh.m_data[0];
    uint pageBaseOffsetInBytes = NaniteMesh.m_data[1 + inPageIndex];
    uint pageBaseOffset = pageBaseOffsetInBytes / 4;
    uint clusterCountOnPage = NaniteMesh.m_data[pageBaseOffset];
    uint clusterBaseOffsetInBytes = NaniteMesh.m_data[pageBaseOffset + 1 + inClusterIndex];
    uint clusterBaseOffset = pageBaseOffset + 1 + clusterCountOnPage + clusterBaseOffsetInBytes / 4;
    uint clusterIndexOffset = NaniteMesh.m_data[clusterBaseOffset] / 4;
    uint clusterIndexCount = NaniteMesh.m_data[clusterBaseOffset + 1];
    uvec4 lodBounds=uvec4(
        NaniteMesh.m_data[clusterBaseOffset + 2u],
        NaniteMesh.m_data[clusterBaseOffset + 3u],
        NaniteMesh.m_data[clusterBaseOffset + 4u],
        NaniteMesh.m_data[clusterBaseOffset + 5u]
    );
    uint lodErrorAndEdgeLength=NaniteMesh.m_data[clusterBaseOffset + 6u];

    ClusterInfo clusterInfo;
    clusterInfo.m_baseOffset=clusterBaseOffset;
    clusterInfo.m_indexOffset=clusterIndexOffset;
    clusterInfo.m_indexCount=clusterIndexCount;
    clusterInfo.m_LODBounds=uintBitsToFloat(lodBounds);
    vec2 unpackedData = unpackHalf2x16(lodErrorAndEdgeLength);
    clusterInfo.m_LODError=unpackedData.x;
    clusterInfo.m_edgeLength=unpackedData.y;

    return clusterInfo;
}

//vec2 GetProjectionScales(vec4 inSphere){
//	vec3 center=inSphere.xyz;
//	float radius=inSphere.w;
//
//	float distanceToSphereCenterSq=dot(center,center);
//	float distanceToSphereCenter=sqrt(distanceToSphereCenterSq);
//
//	float zVS=dot(ViewDirectionWS.xyz,center);
//	float xVS=sqrt(distanceToSphereCenterSq-zVS*zVS);
//
//	float distanceToTangentPoint=sqrt(distanceToSphereCenterSq-radius*radius);
//
//	float sinTheta=radius/distanceToSphereCenter;
//	float cosTheta=distanceToTangentPoint/distanceToSphereCenter;
//
//	float a=(-sinTheta*xVS+cosTheta*zVS)/distanceToSphereCenter;
//	float b=(sinTheta*xVS+cosTheta*zVS)/distanceToSphereCenter;
//
//	float minZ=max(10.0f,zVS-radius);
//	float maxZ=max(10.0f,zVS+radius);
//	if(zVS+radius>10.0f){
//		return vec2(minZ*a,maxZ*b);
//	}
//	return vec2(0.0f,0.0f);
//}

// 基于虚幻源码算法直接修改
vec2 GetProjectedEdgeScales(vec4 Bounds)	// float2(min, max)
{
	if( ProjMatrix[ 3 ][ 3 ] >= 1.0f )
	{
		// Ortho
		return vec2( 1, 1 );
	}
	vec3 Center = Bounds.xyz;
	float Radius = Bounds.w;

	float ZNear = 10.0f;
	float DistToClusterSq = dot( Center, Center );	// camera origin in (0,0,0)
	
	float Z = dot(ViewDirectionWS.xyz, Center);
	float XSq = DistToClusterSq - Z * Z;
	float X = sqrt( max(0.0f, XSq) );
	float DistToTSq = DistToClusterSq - Radius * Radius;
	float DistToT = sqrt( max(0.0f, DistToTSq) );
	float ScaledCosTheta = DistToT;
	float ScaledSinTheta = Radius;
	float ScaleToUnit = 1.0 / DistToClusterSq;
	float By = (  ScaledSinTheta * X + ScaledCosTheta * Z ) * ScaleToUnit;
	float Ty = ( -ScaledSinTheta * X + ScaledCosTheta * Z ) * ScaleToUnit;
	
	float H = ZNear - Z;
	/*if( DistToTSq < 0.0f || By * DistToT < ZNear )
	{
		float Bx = max( X - sqrt( Radius * Radius - H * H ), 0.0f );
		By = ZNear * rsqrt( Bx * Bx + ZNear * ZNear );
	}

	if( DistToTSq < 0.0f || Ty * DistToT < ZNear )
	{	
		float Tx = X + sqrt( Radius * Radius - H * H );
		Ty = ZNear * rsqrt( Tx * Tx + ZNear * ZNear );
	}*/

	float MinZ = max( Z - Radius, ZNear );
	float MaxZ = max( Z + Radius, ZNear );
	float MinCosAngle = Ty;
	float MaxCosAngle = By;

	if(Z + Radius > ZNear)
		return vec2( MinZ * MinCosAngle, MaxZ * MaxCosAngle );
	else
		return vec2( 0.0f, 0.0f );
}

void main()
{
	uint clusterCount = IndirectWorkArgs.m_data[1];	// 获取cluster数量

	uint visibleClusterCount = 0;	// 剔除后的Cluster数量

	// 传递cluster内存分页所在位置
	for (uint index = 0; index < clusterCount; ++index)
	{
    	uint pageIndex = MainAndPostNodeAndClusterBatches.m_data[1024 + index * 2];
		uint clusterIndex = MainAndPostNodeAndClusterBatches.m_data[1024 + index * 2 + 1];
        ClusterInfo clusterInfo = GetClusterInfo(pageIndex, clusterIndex);

        vec3 boundingSphere = clusterInfo.m_LODBounds.xyz;
	    vec4 boundingSphereWS = PerDrawcallBuffer.m_modelMatrix * vec4(boundingSphere, 1.0);
	    boundingSphereWS.xyz = boundingSphereWS.xyz - CameraPositionWS.xyz;	// 

		// 根据 Cluster 的 球形包围盒 计算缩放值
		vec2 projectionScales = GetProjectedEdgeScales(vec4(boundingSphereWS.xyz, clusterInfo.m_LODBounds.w));

		float LODScale = CameraPositionWS.w;
		float LODScaleHW = ViewDirectionWS.w;

		if (projectionScales.x > clusterInfo.m_LODError * LODScale)
		{
			if (projectionScales.x < abs(clusterInfo.m_edgeLength) * LODScaleHW)	// 区分软硬件光栅化
			{
				// HW
			}
			else
			{
				// SW
			}

			VisibleClusterSwHw.m_data[visibleClusterCount].x = MainAndPostNodeAndClusterBatches.m_data[1024 + index * 2];
			VisibleClusterSwHw.m_data[visibleClusterCount].y = MainAndPostNodeAndClusterBatches.m_data[1024 + index * 2 + 1];

			visibleClusterCount++;
		}
	}

	IndirectWorkArgs.m_data[1] = visibleClusterCount;	// 更新Cluster数量
}
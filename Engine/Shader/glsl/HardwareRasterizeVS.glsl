#version 450

layout(binding = 0)uniform GlobalConstants
{
	mat4 ViewProjectionMatrix;
    mat4 ProjMatrix;
	mat4 ViewMatrix;
    mat4 ViewMatrixWithoutTranslate;  // 此ViewMatrix不允许带相机偏移
	uvec4 Misc0;
	vec4 CameraPositionWS;
	vec4 ViewDirectionWS;
};

layout(binding = 1, std430)readonly buffer FNaniteMesh
{
	uint m_data[];
}NaniteMesh;

layout(binding = 2, std430)readonly buffer VisibleClusterSoftwareHardware
{
	uint m_data[];
}VisibleClusterSwHw;

layout(binding = 4, std430)readonly buffer FPerDrawcallBuffer
{
    mat4 m_modelMatrix;
}PerDrawcallBuffer;

layout(location = 0)flat out uvec4 v_cluasterIndex;

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

void main()
{
	uint vertexIndex = gl_VertexID;
	uint clusterIndexWithInvoke = gl_InstanceID;

    uint pageIndex = VisibleClusterSwHw.m_data[clusterIndexWithInvoke * 2];
    uint clusterIndex = VisibleClusterSwHw.m_data[clusterIndexWithInvoke * 2 + 1];

    ClusterInfo clusterInfo = GetClusterInfo(pageIndex,clusterIndex);
    vec4 positionCS = vec4(0.0f,0.0f,0.0f,0.0f);
    if(vertexIndex < clusterInfo.m_indexCount)  // 索引数量不一定有384个
    {
        uint currentVertexIndexOffsetBase = clusterInfo.m_baseOffset + clusterInfo.m_indexOffset;
        uint currentVertexIndexOffset= currentVertexIndexOffsetBase + vertexIndex;
        uint currentIndexInCluster = NaniteMesh.m_data[currentVertexIndexOffset];   // 实际的索引数据
        uint currentClusterPositionOffsetBase = clusterInfo.m_baseOffset + 7u;    // 顶点数据的偏移
        uint currentVertexPositionDataOffset = currentClusterPositionOffsetBase + 3 * currentIndexInCluster;    // 每个顶点数据的偏移
        vec3 positionMS = uintBitsToFloat(
            uvec3(
                NaniteMesh.m_data[currentVertexPositionDataOffset],
                NaniteMesh.m_data[currentVertexPositionDataOffset + 1],
                NaniteMesh.m_data[currentVertexPositionDataOffset + 2]
            )
        );
        vec4 positionWS = PerDrawcallBuffer.m_modelMatrix * vec4(positionMS, 1.0f);
        positionWS.xyz = positionWS.xyz - CameraPositionWS.xyz; // 此算法中，ViewMatrix不能带偏移
        vec4 positionVS = ViewMatrixWithoutTranslate * positionWS;  // 乘上没有Translate的ViewMatrix
        positionCS = ProjMatrix * positionWS;

        v_cluasterIndex.x = (pageIndex << 8) | (clusterIndex + 1);  // ???
    }
    else
    {
        return;
    }

    gl_Position = positionCS;
};
#pragma once
#include <Common.h>
#include <vector>
#include "Runtime/Core/Math/Matrix4.h"
#include "Runtime/Core/Math/AxisAligned.h"

NAMESPACE_XYH_BEGIN

// ��Ⱦʵ����
class RenderEntity
{
public:
    uint32_t  m_instanceId = 0u;
    Matrix4x4 m_modelMatrix = Matrix4x4::IDENTITY;

    // mesh
	size_t m_meshAssetId = 0u;  // ������ԴID
    bool m_enableVertexBlending = false;
	std::vector<Matrix4x4> m_jointMatrices; // �ؽھ���
	AxisAlignedBox m_boundingBox;   // ��Χ��

    // material
	size_t m_materialAssetId = 0u; // ������ԴID
	bool m_blend = false;   // �Ƿ����û��
	bool m_doubleSided = false; // �Ƿ�˫����Ⱦ
	Vector4 m_baseColor_Factor = { 1.0f, 1.0f, 1.0f, 1.0f };    // ������ɫ����
	float m_metallicFactor = 1.0f; // ����������
	float m_roughnessFactor = 1.0f; // �ֲڶ�����
	float m_normalScale = 1.0f;	// ������ͼ��������
	float m_occlusionStrength = 1.0f;	// �ڵ�ǿ��
	Vector3 m_emissiveFactor = { 0.0f, 0.0f, 0.0f };	// �Է�������
};

NAMESPACE_XYH_END
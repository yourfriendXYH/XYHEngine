#pragma once
#include <Common.h>
#include "Runtime/Core/Math/MathHeaders.h"
#include <mutex>

NAMESPACE_XYH_BEGIN

//�������
enum class ERenderCameraType : int
{
	Editor,	// �༭�����
	Motor,	// �˶����
};

class RenderCamera
{
public:
	void SetCurrentCameraType(ERenderCameraType type);

	void SetMainViewMatrix(const Matrix4x4& viewMatrix, ERenderCameraType type = ERenderCameraType::Editor);	// ��������ͼ����

	void Move(Vector3 delta);

	void Rotate(Vector2 delta);	// ��ת

	void Zoom(float offset);	// ����

	void LookAt(const Vector3& position, const Vector3& target, const Vector3& up);	// �۲�Ŀ��

	void SetAspectRatio(float aspectRatio);	// ���ÿ�߱�

	void SetFovX(float fovX);	// ����ˮƽ�ӳ���

	Vector3 Position() const { return m_position; }	// ��ȡλ��

	Quaternion Rotation() const { return m_rotation; }	// ��ȡ��ת

	Vector3 Forward() const { return m_inverseRotation * s_Y; }	// ��ȡǰ������
	Vector3 Up() const { return m_inverseRotation * s_Z; }	// ��ȡ��������
	Vector3 Right() const { return m_inverseRotation * s_X; }	// ��ȡ��������

	Vector2 GetFOV() const { return Vector2(m_fovX, m_fovY); }	// ��ȡ�ӳ���

	Matrix4x4 GetViewMatrix();	// ��ȡ��ͼ����

	Matrix4x4 GetPersProjMatrix() const;	// ��ȡ͸��ͶӰ����

	Matrix4x4 GetLookAtMatrix() const;	// ��ȡ�۲����

	// float GetFovYDeprecated() const { return m_fovY; }	// ��ȡ��ֱ�ӳ��ǣ��ѷ�����

public:
	ERenderCameraType m_currentCameraType{ ERenderCameraType::Editor };
	static const Vector3 s_X, s_Y, s_Z;	// ��������ϵ��
	Vector3 m_position{ Vector3::ZERO };
	Quaternion m_rotation{ Quaternion::IDENTITY };
	Quaternion m_inverseRotation{ Quaternion::IDENTITY };	// ������ת
	float m_zNear{ 1000.0f };	// ���ü���
	float m_zFar{ 0.1f };	// Զ�ü���
	Vector3 m_upAxis{ s_Z };	// �Ϸ���

	static constexpr float MIN_FOV = 10.0f;	// ��С�ӳ���
	static constexpr float MAX_FOV = 89.0f;	// ����ӳ���
	static constexpr int MAIN_VIEW_MATRIX_INDEX = 0;	// ����ͼ��������

	std::vector<Matrix4x4> m_viewMatrices = { Matrix4x4::IDENTITY };	// ��ͼ����

protected:
	float m_aspectRatio = 0.0f;	// ��߱�
	float m_fovX = Degree(89.f).valueDegrees();	// ˮƽ�ӳ���
	float m_fovY = 0.0f;	// ��ֱ�ӳ���

	std::mutex m_viewMatrixMutex;	// ��ͼ���󻥳���
};

inline const Vector3 RenderCamera::s_X = { 1.0f, 0.0f, 0.0f };
inline const Vector3 RenderCamera::s_Y = { 0.0f, 1.0f, 0.0f };
inline const Vector3 RenderCamera::s_Z = { 0.0f, 0.0f, 1.0f };

NAMESPACE_XYH_END
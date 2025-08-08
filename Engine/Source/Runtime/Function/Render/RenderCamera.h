#pragma once
#include <Common.h>
#include "Runtime/Core/Math/MathHeaders.h"
#include <mutex>

NAMESPACE_XYH_BEGIN

//相机类型
enum class ERenderCameraType : int
{
	Editor,	// 编辑器相机
	Motor,	// 运动相机
};

class RenderCamera
{
public:
	void SetCurrentCameraType(ERenderCameraType type);

	void SetMainViewMatrix(const Matrix4x4& viewMatrix, ERenderCameraType type = ERenderCameraType::Editor);	// 设置主视图矩阵

	void Move(Vector3 delta);

	void Rotate(Vector2 delta);	// 旋转

	void Zoom(float offset);	// 缩放

	void LookAt(const Vector3& position, const Vector3& target, const Vector3& up);	// 观察目标

	void SetAspectRatio(float aspectRatio);	// 设置宽高比

	void SetFovX(float fovX);	// 设置水平视场角

	Vector3 Position() const { return m_position; }	// 获取位置

	Quaternion Rotation() const { return m_rotation; }	// 获取旋转

	Vector3 Forward() const { return m_inverseRotation * s_Y; }	// 获取前向向量
	Vector3 Up() const { return m_inverseRotation * s_Z; }	// 获取上向向量
	Vector3 Right() const { return m_inverseRotation * s_X; }	// 获取右向向量

	Vector2 GetFOV() const { return Vector2(m_fovX, m_fovY); }	// 获取视场角

	Matrix4x4 GetViewMatrix();	// 获取视图矩阵

	Matrix4x4 GetPersProjMatrix() const;	// 获取透视投影矩阵

	Matrix4x4 GetLookAtMatrix() const;	// 获取观察矩阵

	// float GetFovYDeprecated() const { return m_fovY; }	// 获取垂直视场角（已废弃）

public:
	ERenderCameraType m_currentCameraType{ ERenderCameraType::Editor };
	static const Vector3 s_X, s_Y, s_Z;	// 世界坐标系轴
	Vector3 m_position{ Vector3::ZERO };
	Quaternion m_rotation{ Quaternion::IDENTITY };
	Quaternion m_inverseRotation{ Quaternion::IDENTITY };	// 逆向旋转
	float m_zNear{ 1000.0f };	// 近裁剪面
	float m_zFar{ 0.1f };	// 远裁剪面
	Vector3 m_upAxis{ s_Z };	// 上方向

	static constexpr float MIN_FOV = 10.0f;	// 最小视场角
	static constexpr float MAX_FOV = 89.0f;	// 最大视场角
	static constexpr int MAIN_VIEW_MATRIX_INDEX = 0;	// 主视图矩阵索引

	std::vector<Matrix4x4> m_viewMatrices = { Matrix4x4::IDENTITY };	// 视图矩阵

protected:
	float m_aspectRatio = 0.0f;	// 宽高比
	float m_fovX = Degree(89.f).valueDegrees();	// 水平视场角
	float m_fovY = 0.0f;	// 垂直视场角

	std::mutex m_viewMatrixMutex;	// 视图矩阵互斥锁
};

inline const Vector3 RenderCamera::s_X = { 1.0f, 0.0f, 0.0f };
inline const Vector3 RenderCamera::s_Y = { 0.0f, 1.0f, 0.0f };
inline const Vector3 RenderCamera::s_Z = { 0.0f, 0.0f, 1.0f };

NAMESPACE_XYH_END
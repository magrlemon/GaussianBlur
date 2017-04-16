#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Windows.h>
#include <xnamath.h>
#include <cmath>

class Camera
{
public:
	Camera();

	//Camera position
	void SetPosition(float x, float y, float z)		{ m_position = XMFLOAT3(x,y,z); }
	void SetPositionXM(FXMVECTOR pos)				{ XMStoreFloat3(&m_position,pos); }

	//Gets
	XMFLOAT3 GetPosition()	const	{ return m_position; }
	XMFLOAT3 GetRight()		const	{ return m_right; }
	XMFLOAT3 GetUp()		const	{ return m_up; }
	XMFLOAT3 GetLook()		const	{ return m_look; }

	XMVECTOR GetPositionXM()	const	{ return XMLoadFloat3(&m_position); }
	XMVECTOR GetRightXM()		const	{ return XMLoadFloat3(&m_right); }
	XMVECTOR GetUpXM()			const	{ return XMLoadFloat3(&m_up); }
	XMVECTOR GetLookXM()		const	{	return XMLoadFloat3(&m_look); }
	
	//Get projection infos
	float GetNearZ()	const		{ return m_nearZ; }
	float GetFarZ()		const		{ return m_farZ; }
	float GetFovY()		const		{ return m_fovY; }
	float GetFovX()		const		{ return atan(m_aspect * tan(m_fovY * 0.5f)) * 2.f; }
	float GetAspect()	const		{ return m_aspect; }

	//Get matrices
	XMMATRIX View()				const	{ return XMLoadFloat4x4(&m_view); }
	XMMATRIX Projection()		const	{ return XMLoadFloat4x4(&m_proj); }
	XMMATRIX ViewProjection()	const	{ return XMLoadFloat4x4(&m_view) * XMLoadFloat4x4(&m_proj); }

	//Configure projection
	void SetLens(float fovY, float ratioAspect, float nearZ, float farZ)
	{
		m_fovY = fovY;
		m_aspect = ratioAspect;
		m_nearZ = nearZ;
		m_farZ = farZ;

		XMStoreFloat4x4(&m_proj,XMMatrixPerspectiveFovLH(m_fovY,m_aspect,m_nearZ,m_farZ));
	}

	//Set view matrix using traditional method: pos, viewPoint, up
	void LookAtXM(FXMVECTOR pos, FXMVECTOR lookAt, FXMVECTOR worldUp);
	void LookAt(XMFLOAT3 &pos, XMFLOAT3 &lookAt, XMFLOAT3 &worldUp);

	//Basic operations
	void Walk(float dist);
	void Strafe(float dist);
	void Pitch(float angle);
	void RotateY(float angle);

	//Update view matrix
	void UpdateView();

private:
	XMFLOAT3	m_right;			//Position and 3 basic axis
	XMFLOAT3	m_up;
	XMFLOAT3	m_look;
	XMFLOAT3	m_position;

	float	m_aspect;				//Projection parameters
	float	m_fovY;
	float	m_nearZ;
	float	m_farZ;

	XMFLOAT4X4	m_view;				//View matrix
	XMFLOAT4X4	m_proj;				//Projection matrix
};

#endif
#include "stdafx.h"
#include "Camera.h"
#include "Player.h"
#include "Scene.h"

CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::Identity();
	GenerateProjectionMatrix(1.01f, 30.0f, ASPECT_RATIO, 90.0f);
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_nMode = 0x00;
	m_iPartitionPos = -1;
	m_Floor = -1;
	m_bUpdateUseRotate = true;
}

CCamera::CCamera(const shared_ptr<CCamera>& pCamera)
{
	if (pCamera)
	{
		*this = *pCamera;
	}
	else
	{
		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Projection = Matrix4x4::Identity();
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode = 0x00;

		m_xmf4FogColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_xmf4FogInfo = XMFLOAT4(1.0f, 10.0f, 0.1f, 1.0f);

		m_iPartitionPos = -1;
		m_Floor = -1;
		m_bUpdateUseRotate = true;
	}
}

CCamera::~CCamera()
{
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	//	XMMATRIX xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	//	XMStoreFloat4x4(&m_xmf4x4Projection, xmmtxProjection);
}

void CCamera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void CCamera::GenerateViewMatrix()
{
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);
}

void CCamera::MultiplyViewProjection()
{
	m_xmf4x4ViewProjection = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4Projection);
}

void CCamera::RegenerateViewMatrix()
{
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);

	GenerateFrustum();
}

void CCamera::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256�� ���
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void**)&m_pcbMappedCamera);

	m_d3dCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbCamera.Get(), ncbElementBytes);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4View;
	XMStoreFloat4x4(&xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4View, &xmf4x4View, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 xmf4x4Projection;
	XMStoreFloat4x4(&xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4Projection, &xmf4x4Projection, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 xmf4x4InverseView;
	XMStoreFloat4x4(&xmf4x4InverseView, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_xmf4x4View))));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4InverseView, &xmf4x4InverseView, sizeof(XMFLOAT4X4));

	XMMATRIX xmmtxInverseViewProjection;
	xmmtxInverseViewProjection = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_xmf4x4ViewProjection)));
	XMFLOAT4X4 xmf4x4InverseViewProjection;
	XMStoreFloat4x4(&xmf4x4InverseViewProjection, xmmtxInverseViewProjection);
	::memcpy(&m_pcbMappedCamera->m_xmf4x4InverseViewProjection, &xmf4x4InverseViewProjection, sizeof(XMFLOAT4X4));

	XMFLOAT4 xmf4Position = XMFLOAT4(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 0.0f);
	::memcpy(&m_pcbMappedCamera->m_xmf4Position, &xmf4Position, sizeof(XMFLOAT4));
	
	::memcpy(&m_pcbMappedCamera->m_xmf4FogColor, &m_xmf4FogColor, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedCamera->m_xmf4FogInfo, &m_xmf4FogInfo, sizeof(XMFLOAT4));
	
	pd3dCommandList->SetGraphicsRootDescriptorTable(0, GetDescriptorHandle());
}

void CCamera::UpdateComputeShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4View;
	XMStoreFloat4x4(&xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4View, &xmf4x4View, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 xmf4x4Projection;
	XMStoreFloat4x4(&xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4Projection, &xmf4x4Projection, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 xmf4x4InverseView;
	XMStoreFloat4x4(&xmf4x4InverseView, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_xmf4x4View))));
	::memcpy(&m_pcbMappedCamera->m_xmf4x4InverseView, &xmf4x4InverseView, sizeof(XMFLOAT4X4));

	XMMATRIX xmmtxInverseViewProjection;
	xmmtxInverseViewProjection = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_xmf4x4ViewProjection)));
	XMFLOAT4X4 xmf4x4InverseViewProjection;
	XMStoreFloat4x4(&xmf4x4InverseViewProjection, xmmtxInverseViewProjection);
	::memcpy(&m_pcbMappedCamera->m_xmf4x4InverseViewProjection, &xmf4x4InverseViewProjection, sizeof(XMFLOAT4X4));

	XMFLOAT4 xmf4Position = XMFLOAT4(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 0.0f);
	::memcpy(&m_pcbMappedCamera->m_xmf4Position, &xmf4Position, sizeof(XMFLOAT4));

	::memcpy(&m_pcbMappedCamera->m_xmf4FogColor, &m_xmf4FogColor, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedCamera->m_xmf4FogInfo, &m_xmf4FogInfo, sizeof(XMFLOAT4));

	pd3dCommandList->SetComputeRootDescriptorTable(0, GetDescriptorHandle());
}


void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera.Get()) { // ComPtr�� ����ϴ��� Unmap�� ���־�� �޸� ���� x
		m_pd3dcbCamera.Get()->Unmap(0, NULL);
	}
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}


void CCamera::Update(XMFLOAT3& xmf3LookAt, float fElapsedTime)
{
	if (shared_ptr<CPlayer> pPlayer = m_pPlayer.lock()) {
		m_Floor = static_cast<int>(floor(pPlayer->GetPosition().y / 4.5f));
	}
	else {
		m_Floor = static_cast<int>(m_xmf3Position.y / 4.5f);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CCamera::GetDescriptorHandle()
{
	return m_d3dCbvGPUDescriptorHandle;
}

void CCamera::SetDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	m_d3dCbvGPUDescriptorHandle = handle;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFirstPersonCamera

CFirstPersonCamera::CFirstPersonCamera(const shared_ptr<CCamera>& pCamera)
	: CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CFirstPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fElapsedTime)
{
	if (shared_ptr<CPlayer> pPlayer = m_pPlayer.lock())
	{
		XMFLOAT3 xmf3Offset = m_xmf3Offset;
		if (m_bUpdateUseRotate) {
			XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
			XMFLOAT3 xmf3Right = pPlayer->GetRightVector();
			XMFLOAT3 xmf3Up = pPlayer->GetUpVector();
			XMFLOAT3 xmf3Look = pPlayer->GetLookVector();
			xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
			xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
			xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

			xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
		}

		XMFLOAT3 xmf3Position = Vector3::Add(pPlayer->GetPosition(), xmf3Offset);
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);
		float fTimeLagScale = (m_fTimeLag) ? fElapsedTime * (1.0f / m_fTimeLag) : 1.0f;
		float fDistance = fLength * fTimeLagScale;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			SetLookAt(xmf3LookAt);
		}
	}
	CCamera::Update(xmf3LookAt, fElapsedTime);
}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x == 0.0f && y == 0.0f)
	{
		return;
	}

	XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	XMFLOAT3 xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);

	m_xmf3Look = Vector3::TransformNormal(xmf3Look, xmmtxRotate);
	m_xmf3Up = Vector3::TransformNormal(xmf3Up, xmmtxRotate);
	m_xmf3Right = Vector3::TransformNormal(xmf3Right, xmmtxRotate);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CThirdPersonCamera

CThirdPersonCamera::CThirdPersonCamera(const shared_ptr<CCamera>& pCamera)
	: CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == FIRST_PERSON_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fElapsedTime)
{
	if (shared_ptr<CPlayer> pPlayer = m_pPlayer.lock())
	{
		XMFLOAT3 xmf3Offset = m_xmf3Offset;
		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		XMFLOAT3 xmf3Right = pPlayer->GetRightVector();
		XMFLOAT3 xmf3Up = pPlayer->GetUpVector();
		XMFLOAT3 xmf3Look = pPlayer->GetLookVector();
		xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

		xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);

		XMFLOAT3 xmf3Position = Vector3::Add(pPlayer->GetPosition(), xmf3Offset);
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);
		float fTimeLagScale = (m_fTimeLag) ? fElapsedTime * (1.0f / m_fTimeLag) : 1.0f;
		float fDistance = fLength * fTimeLagScale;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			SetLookAt(xmf3LookAt);
		}
	}
	CCamera::Update(xmf3LookAt, fElapsedTime);
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	shared_ptr<CPlayer> pPlayer = m_pPlayer.lock();
	XMFLOAT3 xmf3PlayerUp = pPlayer->GetUpVector();
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3PlayerUp);
	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}

CLightCamera::CLightCamera()
	: CCamera()
{
	//m_pLight = nullptr;
}

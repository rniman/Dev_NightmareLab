#pragma once
#include "stdafx.h"
#include "GameFramework.h"
#include "Player.h"
#include "Scene.h"
#include "Shader.h"
#include "PlayerController.h"
#include "Collision.h"
#include "EnvironmentObject.h"
#include "TeleportLocation.h"
#include "ParticleShader.h"
#include "SharedObject.h"

#include "Sound.h"
//#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	_TCHAR* pWavName = (_TCHAR*)pCallbackData;
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
	PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
	PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	SetStatic(false);
	m_pCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_ShadowRender = true;
	m_SelfShadowRender = false;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	//if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

//void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
//{
//	if (dwDirection)
//	{
//		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
//		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
//		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
//		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
//		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
//
//		Move(xmf3Shift, bUpdateVelocity);
//	}
//}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else //�ӷ��� Ȯ���Ǹ� �̰��� �۵��� or ������ ��ġ�� �ű涧
	{
		if(!Vector3::IsZero(xmf3Shift))
		{
			m_xmf3OldPosition = m_xmf3Position;
		}
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		CalculateSpace();
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if(m_bAlive)
		{
			if (x != 0.0f)
			{
				m_fPitch += x;
				if (m_fPitch > +70)
				{
					x -= (m_fPitch - 70);
					m_fPitch = +70;
				}
				if (m_fPitch < -80.0f)
				{
					x -= (m_fPitch + 80.0f);
					m_fPitch = -80.0f;
				}
			}
			if (y != 0.0f)
			{
				m_fYaw += y;
				if (m_fYaw > 360.0f)
				{
					m_fYaw -= 360.0f;
				}
				if (m_fYaw < 0.0f)
				{
					m_fYaw += 360.0f;
				}
			}
			if (z != 0.0f)
			{
				m_fRoll += z;
				if (m_fRoll > +20.0f)
				{
					z -= (m_fRoll - 20.0f);
					m_fRoll = +20.0f;
				}
				if (m_fRoll < -20.0f)
				{
					z -= (m_fRoll + 20.0f);
					m_fRoll = -20.0f;
				}
			}
		}
		else 
		{
			if (x != 0.0f)
			{
				m_fPitch += x;
				if (m_fPitch < -180.0f)
				{
					m_fPitch = 360.0f + m_fPitch;
				}
				if (m_fPitch > 180.0f)
				{
					m_fPitch = -360.0f + m_fPitch;
				}
			}
			if (y != 0.0f)
			{
				m_fYaw += y;
				if (m_fYaw < -180.0f)
				{
					m_fYaw = 360.0f + m_fYaw;
				}
				if (m_fYaw > 180.0f)
				{
					m_fYaw = -360.0f + m_fYaw;
				}
			}
			if (z != 0.0f)
			{
				m_fRoll += z;
				if (m_fRoll < -180.0f)
				{
					m_fRoll = 360.0f + m_fRoll;
				}
				if (m_fRoll > 180.0f)
				{
					m_fRoll = -360.0f + m_fRoll;
				}
			}
		}

		m_pCamera->Rotate(m_fPitch, m_fYaw, m_fRoll);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fElapsedTime)
{
	m_pCamera->Update(m_xmf3Position, fElapsedTime);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fElapsedTime);

	m_pCamera->RegenerateViewMatrix();
}

void CPlayer::CalculateSpace()
{
	m_nWidth = static_cast<int>((m_xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	m_nFloor = static_cast<int>((m_xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	m_nDepth = static_cast<int>((m_xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);
}

shared_ptr<CCamera> CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	shared_ptr<CCamera> pNewCamera;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = make_shared<CFirstPersonCamera>(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = make_shared<CThirdPersonCamera>(m_pCamera);
		break;
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
	}

	return(pNewCamera);
}

shared_ptr<CCamera> CPlayer::ChangeCamera(DWORD nNewCameraMode, float fElapsedTime)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(8.0f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.01f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 1.54f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(0.01f, 100.0f, ASPECT_RATIO,70.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(8.0f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, -5.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 100.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));

	return(m_pCamera);
}

// Picking
void CPlayer::SetPickedObject(const shared_ptr<CGameObject> pGameObject)
{
	m_pPickedObject = pGameObject;
}

void CPlayer::OnUpdateToParent()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;
	
	XMMATRIX xmtxScale = XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(xmtxScale, m_xmf4x4ToParent);
}

void CPlayer::Animate(float fElapsedTime)
{
	if (m_nClientId == -1)
	{
		//return;
	}

	OnUpdateToParent();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CPlayer::AnimateOOBB()
{
	//for (auto oobbOrigin : m_voobbOrigin)
	//{
	//	BoundingOrientedBox newOOBB;
	//	XMMATRIX mtx = XMLoadFloat4x4(&m_xmf4x4World);
	//	oobbOrigin.Transform(newOOBB, mtx);
	//}
}

void CPlayer::Collide(float fElapsedTime, const shared_ptr<CGameObject>& pCollidedObject)
{
	//XMFLOAT3 xmf3Velocity;
	//XMFLOAT3 xmf3NormalOfVelocity = Vector3::Normalize(m_xmf3Velocity);

	//XMFLOAT3 xmf3OldPosition = m_xmf3OldPosition;
	//m_bCollision = false;

	//BoundingBox aabbPlayer;

	//XMFLOAT3 xmf3SubVelocity[3];
	//xmf3SubVelocity[0] = XMFLOAT3(xmf3NormalOfVelocity.x, 0.0f, xmf3NormalOfVelocity.z);
	//xmf3SubVelocity[1] = XMFLOAT3(xmf3NormalOfVelocity.x, 0.0f, 0.0f);
	//xmf3SubVelocity[2] = XMFLOAT3(0.0f, 0.0f, xmf3NormalOfVelocity.z);

	//xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	//float fLength = sqrtf(xmf3Velocity.x * xmf3Velocity.x + xmf3Velocity.z * xmf3Velocity.z);
	//float fMaxVelocityXZ = m_fMaxVelocityXZ;
	//if (fLength > m_fMaxVelocityXZ)
	//{
	//	xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
	//	xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	//}
	//float fMaxVelocityY = m_fMaxVelocityY;
	//fLength = sqrtf(xmf3Velocity.y * xmf3Velocity.y);
	//if (fLength > m_fMaxVelocityY) xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//XMFLOAT3 xmf3ResultVelocity = Vector3::ScalarProduct(xmf3Velocity, fElapsedTime, false);

	//for (int k = 0; k < 3; ++k)
	//{
	//	m_xmf3Position = xmf3OldPosition;
	//	CalculateSpace();

	//	m_bCollision = false;
	//	xmf3SubVelocity[k] = Vector3::ScalarProduct(xmf3SubVelocity[k], Vector3::Length(xmf3ResultVelocity), false);
	//	Move(xmf3SubVelocity[k], false);
	//	m_pCamera->Move(Vector3::ScalarProduct(xmf3SubVelocity[k], -1.0f, false));

	//	OnUpdateToParent();
	//	aabbPlayer.Center = m_voobbOrigin[0].Center;
	//	aabbPlayer.Extents = m_voobbOrigin[0].Extents;
	//	XMVECTOR xmvTranslation = XMVectorSet(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 1.0f);
	//	aabbPlayer.Transform(aabbPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);

	//	for (int i = m_nWidth - 1; i <= m_nWidth + 1 && !m_bCollision; ++i)
	//	{
	//		for (int j = m_nDepth - 1; j <= m_nDepth + 1 && !m_bCollision; ++j)
	//		{
	//			if (i < 0 || i >= g_collisionManager.GetWidth() || j < 0 || j >= g_collisionManager.GetDepth())
	//			{
	//				continue;
	//			}

	//			for (const auto& object : g_collisonManager.GetSpaceGameObjects(m_nFloor, i, j))
	//			{
	//				shared_ptr<CGameObject> pGameObject = object.lock();
	//				if (!pGameObject || pGameObject->GetCollisionType() == 2)	//�ӽ÷� 2�� �ѱ�
	//				{
	//					continue;
	//				}

	//				for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
	//				{
	//					BoundingOrientedBox oobb;
	//					oobbOrigin.Transform(oobb, XMLoadFloat4x4(&pGameObject->m_xmf4x4World));
	//					XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

	//					if (oobb.Intersects(aabbPlayer))
	//					{ 
	//						m_bCollision = true;
	//						break;
	//					}
	//				}

	//				if (m_bCollision)
	//				{
	//					break;
	//				}
	//			}
	//		}
	//	}
	//	if (!m_bCollision)
	//	{
	//		if(!Vector3::IsZero(xmf3SubVelocity[k]))
	//		{
	//			m_xmf3OldVelocity = xmf3SubVelocity[k];
	//		}
	//		break;
	//	}
	//}
	//
	//if (m_bCollision)
	//{
	//	m_xmf3Position = m_xmf3OldPosition = xmf3OldPosition;
	//	//m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//	CalculateSpace();
	//}

	//DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//m_pCamera->Update(m_xmf3Position, fElapsedTime);
	//if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fElapsedTime);
	//m_pCamera->RegenerateViewMatrix();
	//
	//OnUpdateToParent();
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nClientId == -1)
	{
		return;
	}

	if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA || m_nClientId != CGameFramework::GetMainClientId() || !m_bAlive)
	{
		CGameObject::Render(pd3dCommandList);
	}
	else  {
		CGameObject::Render(pd3dCommandList, (char*)"HeadF");
	}
}

void CPlayer::SetPlayerVolume(float fPlayerVolume)
{
	m_fPlayerVolume = fPlayerVolume;
	m_pSkinnedAnimationController->SetPlayerVolume(fPlayerVolume);
}

void CPlayer::SetGameStart()
{
	m_bGameStartWait = true;
	m_fGameStartCount = 10.f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CBlueSuitPlayer::CBlueSuitPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
	:CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext)
{
	m_xmf2RadarUIPos = XMFLOAT2{ 0.0f,0.0f };

	m_pCamera->SetFogColor(XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f));
	m_pCamera->SetFogInfo(XMFLOAT4(0.0f, 10.0f, 0.05f, 1.0f));

	//m_xmf3Scale = XMFLOAT3(1.25f,1.25f,1.25f);

	m_xmf4x4Rader = Matrix4x4::Identity();
	/*m_pRader = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pTeleport = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pMine = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pFuse = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);*/

	m_apSlotItems[0] = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_apSlotItems[0]->SetCollision(false);
	m_apSlotItems[1] = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_apSlotItems[1]->SetCollision(false);
	m_apSlotItems[2] = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_apSlotItems[2]->SetCollision(false);

	for (int i = 0; i < 3; ++i)
	{
		m_apFuseItems[i] = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
		m_apFuseItems[i]->SetCollision(false);
	}

	shared_ptr<CTexture> pTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/Textures/redColor.dds", RESOURCE_TEXTURE2D, 0);
	m_pHitEffectMaterial = make_shared<CMaterial>(1); // �ؽ�ó�� 1��
	m_pHitEffectMaterial->SetTexture(pTexture, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 13);

	UINT ncbElementBytes = ((sizeof(FrameTimeInfo) + 255) & ~255); //256�� ���
	m_pd3dcbTime = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTime->Map(0, NULL, (void**)&m_pcbMappedTime);
	m_d3dTimeCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTime.Get(), ncbElementBytes);
	m_pcbMappedTime->usePattern = -1.0f;

	m_pd3dcbTimeEnd = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTimeEnd->Map(0, NULL, (void**)&m_pcbMappedTimeEnd);
	m_d3dTimeCbvGPUDescriptorHandleEnd = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTimeEnd.Get(), ncbElementBytes);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

}

CBlueSuitPlayer::~CBlueSuitPlayer()
{
}

void CBlueSuitPlayer::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);
	LoadBoundingBox(m_voobbOrigin);
	int nCbv = 0;
	nCbv = cntCbvModelObject(shared_from_this(), 0);

	m_pSkinnedAnimationController = make_shared<CBlueSuitAnimationController>(pd3dDevice, pd3dCommandList, 6, pLoadModelInfo);

	//	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
	//#ifdef _WITH_SOUND_RESOURCE
	//	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	//	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	//	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
	//#else
	//	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.1f, _T("Asset/Sound/Footstep01.wav"));
	//	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.9f, _T("Asset/Sound/Footstep02.wav"));
	//#endif
	//	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	//	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);
}

shared_ptr<CCamera> CBlueSuitPlayer::ChangeCamera(DWORD nNewCameraMode, float fElapsedTime)
{
	shared_ptr<CCamera> camera = CPlayer::ChangeCamera(nNewCameraMode, fElapsedTime);
	if (camera->GetMode() != THIRD_PERSON_CAMERA) {
		int index = dynamic_pointer_cast<CBlueSuitAnimationController>(m_pSkinnedAnimationController)->GetBoneFrameIndex((char*)"Head_M");
		XMFLOAT3 offset = m_pSkinnedAnimationController->GetBoneFramePositionVector(index);
		offset.x = 0.1f; offset.z = 0.2f;
		offset.y = offset.y - m_xmf3Position.y+  0.1f;	// [0507]����
		camera->SetOffset(offset);
		camera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	}

	return camera;
}

void CBlueSuitPlayer::Rotate(float x, float y, float z)
{
	CPlayer::Rotate(x, y, z);
}

//void CBlueSuitPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
//{
//	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
//
//	if ((dwDirection & LSHIFT) && m_bAbleRun) 
//	{
//		m_bShiftRun = true;
//	}
//	else
//	{
//		m_bShiftRun = false;
//	}
//
//	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
//}

void CBlueSuitPlayer::Update(float fElapsedTime)
{
	if (m_bInterruption)
	{
		m_fInterruption += fElapsedTime;
		if (m_fInterruption > 2.0f)
		{
			m_fInterruption = 2.0f;
		}
	}
	else
	{
		m_fInterruption -= fElapsedTime;
		if (m_fInterruption <= 0.0f)
		{
			m_fInterruption = 0.0f;
		}
	}
	m_pCamera->SetFogInfo(XMFLOAT4(0.0f, 10.0f, 0.05f + m_fInterruption / 8, 1.0f));

	if (m_bRunning)
	{
		m_fStamina -= fElapsedTime;
		m_fFullStaminaTime = 0.0f;
	}
	else if (m_fStamina < BLUESUIT_STAMINA_MAX)
	{
		m_fStamina += fElapsedTime;
		if (m_fStamina > BLUESUIT_STAMINA_MAX)
		{
			m_fStamina = BLUESUIT_STAMINA_MAX;
		}
	}
	else
	{
		m_fFullStaminaTime += fElapsedTime;
	}

	if (m_bTeleportUse) {
		XMFLOAT3 position = GetPosition();
		position.y += 1.5f;
		sharedobject.m_vParticleObjects[int(CParticleMesh::TP)]->SetParticlePosition(m_iTeleportParticleId,position);

		if (gGameTimer.GetTotalTime() - 2.8f > m_fCreateParticleTime) {
			m_bTeleportUse = false;
			m_iTeleportParticleId = -1;
		}
	}

	if (m_pZombiePlayer) {
		XMFLOAT3 xmf3Zompos = m_pZombiePlayer->GetPosition();
		if (abs(xmf3Zompos.y - m_xmf3Position.y) < 5.0f) { // ������ 1��������
			XMFLOAT3 xmf3betweenPos = Vector3::Subtract(xmf3Zompos, m_xmf3Position);

			if (Vector3::Length(xmf3betweenPos) > 15.0f) {
				m_bSense = false;
			}
			else {
				m_bSense = true;
			}
		}
		else {
			m_bSense = false;
		}
	}

	m_pCamera->GenerateProjectionMatrix(0.01f, 100.0f, ASPECT_RATIO, 60.0f);

	// ī�޶� ��ġ ������Ʈ
	CPlayer::Update(fElapsedTime);
}

void CBlueSuitPlayer::UpdateAnimation()
{
	if (m_pSkinnedAnimationController && m_pSkinnedAnimationController->IsAnimation())
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (!m_bAlive)
		{
			if (!m_pSkinnedAnimationController->m_bTransition)
			{
				if (m_pSkinnedAnimationController->m_nNowState != PlayerState::DEATH)
				{
					m_pSkinnedAnimationController->m_bTransition = true;
					m_pSkinnedAnimationController->m_nNextState = PlayerState::DEATH;

					//�״� ����
					SoundManager& soundManager = soundManager.GetInstance();
					//soundManager.SetVolume(sound::DEAD_BLUESUIT, m_fPlayerVolume * 0.1f);
					if (m_fPlayerVolume - EPSILON >= 0.0f) soundManager.PlaySoundWithName(sound::DEAD_BLUESUIT, m_fPlayerVolume * 0.1f);
				}
			}
		}
		else if (::IsZero(fLength))	//�ӷ��� 0�̸� Ʈ�� 0�� �ٽ� true
		{
			if (m_pSkinnedAnimationController->m_nNowState != PlayerState::IDLE)
			{
				if (!m_pSkinnedAnimationController->m_bTransition)
				{
					m_pSkinnedAnimationController->m_bTransition = true;
					if (m_pSkinnedAnimationController->m_nNowState == PlayerState::RUN)
					{
						m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
						m_pSkinnedAnimationController->SetTrackPosition(2, m_pSkinnedAnimationController->m_vAnimationTracks[3].m_fPosition);
						m_pSkinnedAnimationController->SetTrackPosition(1, m_pSkinnedAnimationController->m_vAnimationTracks[3].m_fPosition);
					}
					else
					{
						m_pSkinnedAnimationController->m_nNextState = PlayerState::IDLE;
					}
				}
			}
		}
		else
		{
			if (m_pSkinnedAnimationController->m_nNowState == PlayerState::IDLE)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
			}
			else if (m_bRunning && m_pSkinnedAnimationController->m_nNowState == PlayerState::WALK)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::RUN;
				m_pSkinnedAnimationController->SetTrackPosition(3, m_pSkinnedAnimationController->m_vAnimationTracks[1].m_fPosition);
			}
			else if (!m_bRunning && m_pSkinnedAnimationController->m_nNowState == PlayerState::RUN)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
				m_pSkinnedAnimationController->SetTrackPosition(2, m_pSkinnedAnimationController->m_vAnimationTracks[3].m_fPosition);
				m_pSkinnedAnimationController->SetTrackPosition(1, m_pSkinnedAnimationController->m_vAnimationTracks[3].m_fPosition);
			}

			XMFLOAT3 xmf3Direction = Vector3::Normalize(m_xmf3Velocity);
			float fAngle = Vector3::Angle(m_xmf3Look, xmf3Direction);
			float fRightWeight;

			if (Vector3::CrossProduct(m_xmf3Look, m_xmf3Velocity, false).y < 0.0f)	// �����̵�
			{
				m_pSkinnedAnimationController->SetTrackSpeed(2, -1.0f);
			}
			else // ������ �̵�
			{
				m_pSkinnedAnimationController->SetTrackSpeed(2, 1.0f);
			}

			if (fAngle <= 90.0f)
			{
				m_pSkinnedAnimationController->SetTrackSpeed(1, 1.0f);
				fRightWeight = fAngle / 90.0f;
			}
			else
			{
				m_pSkinnedAnimationController->SetTrackSpeed(1, -1.0f);
				fRightWeight = 1 - ((fAngle - 90.0f) / 90.0f);
			}

			// RightWeight 1 �̸� ������ �ִϸ��̼�(Track2)�� ���
			m_pSkinnedAnimationController->SetBlendWeight(0, fRightWeight);
		}
	}
}

void CBlueSuitPlayer::UpdateEnding(float fEndingElapsedTime, int nGameState)
{
	switch (nGameState)
	{
	case GAME_STATE::BLUE_SUIT_WIN:
		if (!m_bAlive)
		{
			// �ִ� �ð��� ������ �� �̻� �������� �ʵ��� ����
			float fMaxFogTime = 3.0f; // �Ȱ��� �ִ�ġ�� �����ϴ� �ð� (�� ����)
			if (fEndingElapsedTime > fMaxFogTime)
			{
				fEndingElapsedTime = fMaxFogTime;
			}

			float fFogDensity = 1 - fEndingElapsedTime / fMaxFogTime;

			XMFLOAT4 xmf4EndingFogInfo = XMFLOAT4(1.0f, 0.0f, 0.0f, fFogDensity);
			XMFLOAT4 xmf4EndingFogColor = XMFLOAT4(0.1f + fEndingElapsedTime * 0.15f, 0.1f, 0.1f, 0.1f);
			m_pCamera->SetFogColor(xmf4EndingFogColor);
			m_pCamera->SetFogInfo(xmf4EndingFogInfo);
		}
		else
		{
			// �ִ� �ð��� ������ �� �̻� �������� �ʵ��� ����
			float fMaxFogTime = 3.0f; // �Ȱ��� �ִ�ġ�� �����ϴ� �ð� (�� ����)
			if (fEndingElapsedTime > fMaxFogTime)
			{
				fEndingElapsedTime = fMaxFogTime;
			}

			float fFogDensity = 1 - fEndingElapsedTime / fMaxFogTime;

			XMFLOAT4 xmf4EndingFogInfo = XMFLOAT4(0.1f, 0.0f, 1.0f, fFogDensity);
			XMFLOAT4 xmf4EndingFogColor = XMFLOAT4(0.1f + fEndingElapsedTime, 0.1f + fEndingElapsedTime, 0.1f + fEndingElapsedTime, 0.1f + fEndingElapsedTime);
			m_pCamera->SetFogColor(xmf4EndingFogColor);
			m_pCamera->SetFogInfo(xmf4EndingFogInfo);
		}
		break;
	case GAME_STATE::ZOMBIE_WIN:
	{
		// �ִ� �ð��� ������ �� �̻� �������� �ʵ��� ����
		float fMaxFogTime = 3.0f; // �Ȱ��� �ִ�ġ�� �����ϴ� �ð� (�� ����)
		if (fEndingElapsedTime > fMaxFogTime)
		{
			fEndingElapsedTime = fMaxFogTime;
		}

		// �ð��� ���� �Ȱ� �е� ��� (0.0���� 1.0���� ����)
		float fFogDensity = 1 - fEndingElapsedTime / fMaxFogTime;

		XMFLOAT4 xmf4EndingFogInfo = XMFLOAT4(1.0f, 0.0f, 0.0f, fFogDensity);
		XMFLOAT4 xmf4EndingFogColor = XMFLOAT4(0.1f + fEndingElapsedTime * 0.15f, 0.1f, 0.1f, 0.1f);
		m_pCamera->SetFogColor(xmf4EndingFogColor);
		m_pCamera->SetFogInfo(xmf4EndingFogInfo);
	}
		break;
	default:
		break;
	}
}

void CBlueSuitPlayer::Animate(float fElapsedTime)
{
	if (!m_pSkinnedAnimationController->IsAnimation())	// [0507] ���� �÷��̾�� Animate�������
	{
		return;
	}
	UpdateAnimation();

	auto controller = dynamic_pointer_cast<CBlueSuitAnimationController>(m_pSkinnedAnimationController);
	if (controller) {
		controller->SetElbowPitch(m_fPitch);
		if (m_selectItem != RightItem::NONE) {
			controller->SetSelectItem(true);
		}
		else {
			controller->SetSelectItem(false);
		}
	}

	CPlayer::Animate(fElapsedTime);

	//�÷��ö���Ʈ
	if(m_pFlashlight) m_pFlashlight->UpdateTransform(GetLeftHandItemFlashLightModelTransform());
	//�÷��̾��� �޼�: ���̴� // �� �������� ���̴��� Ȯ��Ǵ°�ó�� ����. ��, �ڽ��� �����տ� ���̴��� ������� Ȯ��â����.
	// �ٸ� �÷��̾ ������ �Ҷ��� �տ� �ִ� ���·� ��������.

	if (m_pRader) m_pRader->SetObtain(true);
	if (m_pTeleport) m_pTeleport->SetObtain(true);
	if (m_pMine) m_pMine->SetObtain(true);
	if (m_pFuse) m_pFuse->SetObtain(true);

	switch (m_selectItem)
	{
	case RightItem::NONE:
		break;
	case RightItem::RADAR: {
		m_pRader->SetObtain(false);
		m_pRader->UpdateTransform(RadarUpdate(fElapsedTime));
		break;
	}
	case RightItem::TELEPORT:
		m_pTeleport->SetObtain(false);
		m_pTeleport->UpdateTransform(GetRightHandItemTeleportItemModelTransform());
		break;
	case RightItem::LANDMINE:
		m_pMine->SetObtain(false);
		m_pMine->UpdateTransform(GetRightHandItemTeleportItemModelTransform());
		break;
	case RightItem::FUSE:
		m_pFuse->SetObtain(false);
		m_pFuse->UpdateTransform(GetRightHandItemTeleportItemModelTransform());
		break;
	default:
		break;
	}

	m_pcbMappedTime->time = fElapsedTime;
	m_pcbMappedTime->localTime += fElapsedTime;
	if (int(m_pcbMappedTime->localTime * 10.0f) % 3 == 0) { // ��Ʈ�� ������
		m_pcbMappedTime->usePattern = -1.0f;
	}
	else {
		m_pcbMappedTime->usePattern = 1.0f;
	}
	if (m_pcbMappedTime->localTime >= 1.5f) {
		m_pcbMappedTime->usePattern = -1.0f;
		m_bHitEffectBlend = false;
	}
	
	if (m_bTracking)
	{
		m_pcbMappedTime->fTrackingTime += fElapsedTime;
		if (m_pcbMappedTime->fTrackingTime > 2.0f)
		{
			m_pcbMappedTime->fTrackingTime = 2.0f;
		}
	}
	else
	{
		m_pcbMappedTime->fTrackingTime -= fElapsedTime;
		if (m_pcbMappedTime->fTrackingTime < 0.0f)
		{
			m_pcbMappedTime->fTrackingTime = 0.0f;
		}
	}
}

void CBlueSuitPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nClientId == -1 || m_SelfShadowRender)
	{
		return;
	}

	if (m_ShadowRender) {
		CGameObject::Render(pd3dCommandList);
		return;
	}

	if (m_bHitEffectBlend)
	{
		m_pHitEffectMaterial->UpdateShaderVariable(pd3dCommandList, nullptr);

		pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);

		CPlayer::Render(pd3dCommandList);

		pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandleEnd);
		return;
	}

	if(m_bTracking)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);
	}
	CPlayer::Render(pd3dCommandList);

}

void CBlueSuitPlayer::MainPlayerRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::Render(pd3dCommandList);

}

void CBlueSuitPlayer::UpdatePicking() 
{
	//shared_ptr<CGameObject> pPickedObject = m_pPickedObject.lock();

	//if(AddItem(pPickedObject) != -2)
	//{
	//	pPickedObject->UpdatePicking();
	//}
}

void CBlueSuitPlayer::RightClickProcess(float fCurTime)
{
	switch (m_selectItem)
	{
	case RightItem::NONE:
		break;
	case RightItem::RADAR:
		m_bRightClick = !m_bRightClick;
		m_fOpenRadarTime = 0.3f;
		break;
	case RightItem::TELEPORT:
		break;
	case RightItem::LANDMINE:
		break;
	case RightItem::FUSE:
		break;
	default:
		break;
	}
}

int CBlueSuitPlayer::AddItem(const shared_ptr<CGameObject>& pGameObject)
{
	int nSlot = -2;
	if (dynamic_pointer_cast<CTeleportObject>(pGameObject))
	{
		nSlot = 0;
	}
	else if (dynamic_pointer_cast<CRadarObject>(pGameObject))
	{
		nSlot = 1;
	}
	else if (dynamic_pointer_cast<CMineObject>(pGameObject))
	{
		nSlot = 2;
	}
	else if (dynamic_pointer_cast<CFuseObject>(pGameObject))
	{
		shared_ptr<CFuseObject> pFuseObject = dynamic_pointer_cast<CFuseObject>(pGameObject);
		if (pFuseObject->IsObtained())
		{
			return nSlot;
		}

		if (m_nFuseNum < 3)
		{
			m_apFuseItems[m_nFuseNum].reset();
			m_apFuseItems[m_nFuseNum] = pFuseObject;
			m_nFuseNum++;
			nSlot = -1;
		}
	}
	else
	{
		nSlot = -1;
	}

	if (nSlot <= -1)
	{
		return nSlot;
	}

	if (m_apSlotItems[nSlot])
	{

	}
	else
	{
		m_apSlotItems[nSlot].reset();

		// �ӽ÷�
		switch (nSlot)
		{
		case 0:
			m_apSlotItems[nSlot] = dynamic_pointer_cast<CTeleportObject>(pGameObject);
			break;
		case 1:
			m_apSlotItems[nSlot] = dynamic_pointer_cast<CRadarObject>(pGameObject);
			break;
		case 2:
			m_apSlotItems[nSlot] = dynamic_pointer_cast<CFuseObject>(pGameObject);
			break;
		default:
			break;
		}
	}

	return nSlot;
}

void CBlueSuitPlayer::UseItem(int nSlot)
{
	if (nSlot == 3)
	{
		UseFuse();
	}
	else if (m_apSlotItems[nSlot])
	{
		m_apSlotItems[nSlot]->UpdateUsing(shared_from_this());
		m_apSlotItems[nSlot].reset();
	}
}

void CBlueSuitPlayer::UseFuse() 
{
	for (auto& fuseItem : m_apFuseItems)
	{
		if(fuseItem)
		{
			fuseItem->UpdateUsing(shared_from_this());
			fuseItem.reset();
		}
	}
	m_nFuseNum = 0;
}

void CBlueSuitPlayer::Teleport()
{
	if (m_fCreateParticleTime != 0.0f) {
		if (gGameTimer.GetTotalTime() - 3.0f < m_fCreateParticleTime) {
			return;
		}
	}
	SoundManager& soundManager = soundManager.GetInstance();
	//soundManager.SetVolume(sound::USE_TP_BLUESUIT, m_fPlayerVolume);
	if (m_fPlayerVolume - EPSILON >= 0.0f) soundManager.PlaySoundWithName(sound::USE_TP_BLUESUIT, m_fPlayerVolume);

	m_fCreateParticleTime = gGameTimer.GetTotalTime();

	XMFLOAT3 position = GetPosition();
	position.y += 1.5f;
	m_iTeleportParticleId = sharedobject.m_vParticleObjects[int(CParticleMesh::TP)]->SetParticleInsEnable(-1, true, gGameTimer.GetTotalTime(), position);
	if (m_iTeleportParticleId != -1) {
		m_bTeleportUse = true;
	}
}

void CBlueSuitPlayer::SetSlotItem(int nIndex, int nReferenceObjectNum)
{
	//if (!m_apSlotItems[nIndex]->IsObtained())
	//{
	//	SoundManager& soundManager = soundManager.GetInstance();
	//	soundManager.PlaySoundWithName(sound::GET_ITEM_BLUESUIT);
	//}
	m_apSlotItems[nIndex]->SetObtain(true);
	m_apSlotItems[nIndex]->SetReferenceNumber(nReferenceObjectNum);
}

void CBlueSuitPlayer::SetSlotItemEmpty(int nIndex)
{
	m_apSlotItems[nIndex]->SetObtain(false);
	m_apSlotItems[nIndex]->SetReferenceNumber(-1);
}

void CBlueSuitPlayer::SetFuseItem(int nIndex, int nReferenceObjectNum)
{
	//if (!m_apFuseItems[nIndex]->IsObtained())
	//{
	//	SoundManager& soundManager = soundManager.GetInstance();
	//	soundManager.PlaySoundWithName(sound::GET_ITEM_BLUESUIT);
	//}
	m_apFuseItems[nIndex]->SetObtain(true);
	m_apFuseItems[nIndex]->SetReferenceNumber(nReferenceObjectNum);
}

 void CBlueSuitPlayer::SetFuseItemEmpty(int nIndex)
{
	m_apFuseItems[nIndex]->SetObtain(false);
	m_apFuseItems[nIndex]->SetReferenceNumber(-1);
}

XMFLOAT4X4* CBlueSuitPlayer::GetLeftHandItemFlashLightModelTransform() const
{
	auto controller = m_pSkinnedAnimationController.get();
	int i = dynamic_pointer_cast<CBlueSuitAnimationController>(m_pSkinnedAnimationController)->GetBoneFrameIndexToFlashLight();

	return &controller->m_pAnimationSets->m_vpBoneFrameCaches[i]->m_xmf4x4World;
}

void CBlueSuitPlayer::SetHitEvent()
{
	if (!m_bHitEffectBlend) {
		m_pcbMappedTime->localTime = 0.0f;
		m_pcbMappedTime->usePattern = 1.0f;
		m_bHitEffectBlend = true;

		SetHitRender(true);
	}
}

void CBlueSuitPlayer::RenderTextUI(ComPtr<ID2D1DeviceContext2>& d2dDeviceContext, ComPtr<IDWriteTextFormat>& textFormat, ComPtr<ID2D1SolidColorBrush>& brush)
{
	wchar_t text[128]; // ��ȯ�� �����ڵ� ���ڿ��� ������ ����

	//// ȭ�� �߾ӿ� ���� ������
	//POINT windowSize = CGameFramework::GetClientWindowSize();
	////������ ��ǥ ����.
	//D2D1_RECT_F TimetextRect = D2D1::RectF(0.f, 0.f, (float)windowSize.x, (float)windowSize.y);
	//int length = swprintf(text, 43, L"%d:%d", ((int)gGameTimer.GetTotalTime()) / 60, ((int)gGameTimer.GetTotalTime()) % 60);
	//text[length + 1] = '\0';
	//d2dDeviceContext->DrawText(
	//	text,
	//	/*_countof(text)*/length,
	//	CGameFramework::m_idwSpeakerTextFormat.Get(),
	//	&TimetextRect,
	//	brush.Get()
	//);

	if (m_bGameStartWait) {
		m_fGameStartCount -= gGameTimer.GetTimeElapsed();
		if (m_fGameStartCount <= 0.0f) {
			m_bGameStartWait = false;
		}
		int len = swprintf(text, 43, L"����� �����ڰ� �ൿ�� �����մϴ�.\nǻ� ��� Ż�ⱸ�� ã������.");
		text[len + 1] = '\0';

		POINT pos = CGameFramework::GetClientWindowSize();
		D2D1_RECT_F textRect = D2D1::RectF(0, 0, pos.x, pos.y / 2);

		auto color = brush->GetColor();
		brush->SetColor(D2D1::ColorF(D2D1::ColorF::DarkGray));

		if (m_fGameStartCount <= 3.0f) {
			float curTime = gGameTimer.GetTotalTime();
			brush->SetOpacity(curTime - floor(curTime));
		}

		d2dDeviceContext->DrawText(
			text,
			/*_countof(text)*/len,
			CGameFramework::m_idwSpeakerTextFormat.Get(),
			&textRect,
			brush.Get()
		);
		brush->SetColor(color);
		brush->SetOpacity(1.0f);
	}



	if (PlayRadarUI()) {
		float escapelength = GetEscapeLength();
		// �ε� �Ҽ��� ���� ���ڿ��� ��ȯ �� �����ڵ� ���ڿ��� ����
		int len = swprintf(text, 20, L"%d", (int)escapelength);
		text[len] = 'm';
		len += 1;
		text[len + 1] = '\0';
		//D2D1::Matrix3x2F mat = D2D1::Matrix3x2F::Identity();
		//mat = mat.Translation(point.x,point.y);
		//m_d2dDeviceContext->SetTransform(mat);
		static XMFLOAT2 RadarOffset = { 80.0f,30.0f };
		XMFLOAT2 point = GetRadarWindowScreenPos();
		//������ ��ǥ ����.
		D2D1_RECT_F textRect = D2D1::RectF(30.0f + point.x - RadarOffset.x, point.y - RadarOffset.y, point.x + RadarOffset.x, point.y + RadarOffset.y);
		d2dDeviceContext->DrawText(
			text,
			/*_countof(text)*/len,
			textFormat.Get(),
			&textRect,
			brush.Get()
		);
		//textFormat->set
	}
}

XMFLOAT4X4 CBlueSuitPlayer::GetRightHandItemRaderModelTransform() const
{
	auto controller = m_pSkinnedAnimationController.get();
	int i = dynamic_pointer_cast<CBlueSuitAnimationController>(m_pSkinnedAnimationController)->GetBoneFrameIndexToRightHandRaderItem();

	return controller->m_pAnimationSets->m_vpBoneFrameCaches[i]->m_xmf4x4World;
}

XMFLOAT4X4* CBlueSuitPlayer::GetRightHandItemTeleportItemModelTransform() const
{
	auto controller = m_pSkinnedAnimationController.get();
	int i = dynamic_pointer_cast<CBlueSuitAnimationController>(m_pSkinnedAnimationController)->GetBoneFrameIndexToRightHandTeleportItem();

	return &controller->m_pAnimationSets->m_vpBoneFrameCaches[i]->m_xmf4x4World;
}


XMFLOAT4X4* CBlueSuitPlayer::RadarUpdate(float fElapsedTime)
{
	if (m_bRightClick) {
		if (m_fOpenRadarTime > 0.0f) {
			m_fOpenRadarTime -= fElapsedTime;
			if (m_fOpenRadarTime < 0.0f) {
				m_fOpenRadarTime = 0.0f;
			}
		}
		m_xmf4x4Rader = Matrix4x4::Identity();
		// �÷��̾��� ī�޶� ��ġ ��������
		XMFLOAT3 cPos = GetCamera()->GetPosition();
		XMVECTOR pos = { cPos.x, cPos.y, cPos.z, 1.0f };

		// �÷��̾��� ī�޶� �ٶ󺸴� ���� ���� ��������
		XMFLOAT3 cLook = GetCamera()->GetLookVector();
		XMVECTOR look = { cLook.x, cLook.y, cLook.z, 1.0f };

		// ī�޶��� Right, Up ����
		XMFLOAT3 cRight = GetCamera()->GetRightVector();
		XMVECTOR right = { cRight.x, cRight.y, cRight.z, 1.0f };

		XMFLOAT3 cUp = GetCamera()->GetUpVector();
		XMVECTOR up = { cUp.x, cUp.y, cUp.z, 1.0f };

		// ī�޶� �ٶ󺸴� �������� �̵��� ���� ���
		XMVECTOR translation = XMVectorScale(look, 0.5f);
		right = XMVectorScale(right, 0.25f);
		up = XMVectorScale(up, m_fOpenRadarTime + 0.1f);
		translation += right - up;

		// Right, Up, Look ���ͷ� m_xmf4x4Rader ��� ������Ʈ
		m_xmf4x4Rader._11 = cRight.x; m_xmf4x4Rader._12 = cRight.y; m_xmf4x4Rader._13 = cRight.z;
		m_xmf4x4Rader._21 = cUp.x; m_xmf4x4Rader._22 = cUp.y; m_xmf4x4Rader._23 = cUp.z;
		m_xmf4x4Rader._31 = cLook.x; m_xmf4x4Rader._32 = cLook.y; m_xmf4x4Rader._33 = cLook.z;
		
		// m_xmf4x4Rader ����� ��ȯ�Ͽ� ������Ʈ
		XMStoreFloat4x4(&m_xmf4x4Rader,
			(XMMatrixRotationZ(XMConvertToRadians(175.0f)) * XMMatrixRotationX(XMConvertToRadians(90.0f)))
			* XMLoadFloat4x4(&m_xmf4x4Rader)
			* XMMatrixTranslationFromVector(translation + pos));

		// ���̴��� ��ũ�� ��ǥ ���
		XMVECTOR radarWorld = XMVECTOR{ m_xmf4x4Rader._41, m_xmf4x4Rader._42, m_xmf4x4Rader._43, 1.0f };
		m_pCamera->MultiplyViewProjection();
		XMFLOAT4X4 xmf4x4viewprojection = m_pCamera->GetViewProjection();
		XMMATRIX viewprojection = XMLoadFloat4x4(&xmf4x4viewprojection);
		radarWorld = XMVector4Transform(radarWorld, viewprojection);
		XMFLOAT4 temp;
		XMStoreFloat4(&temp, radarWorld);
		float x = temp.x / temp.w;
		float y = temp.y / temp.w;
		POINT windowSize = CGameFramework::GetClientWindowSize();
		m_xmf2RadarUIPos.x = (x + 1.0f) * 0.5f * windowSize.x;
		m_xmf2RadarUIPos.y = (1.0f - y) * 0.5f * windowSize.y;
		//screenPos.x	1165.60400
		//screenPos.y	658.242798
	}
	else {
		m_xmf4x4Rader = GetRightHandItemRaderModelTransform();
	}

	return &m_xmf4x4Rader;
}

float CBlueSuitPlayer::GetEscapeLength()
{
	XMFLOAT3 pos = GetPosition();
	XMFLOAT3 escapePosToPos = Vector3::Subtract(pos, m_fEscapePos);
	float length = Vector3::Length(escapePosToPos);

	return length;
}

void CBlueSuitPlayer::AddEnvironmentMineItems(shared_ptr<CMineObject> object)
{
	m_vpEnvironmentMineItems.push_back(object);
}

void CBlueSuitPlayer::UseMine(int item_id)
{
	// ���� �������� ������ �ٽ� �������� �ʾƾ� ��.
	// ���ڸ� ��ġ�ϰ� �� ���ڸ� ���� ������ �������� ����� ���� ����.
	// ���� �������� �Ծ��ٸ� Ŭ���̾�Ʈ���� ������ ���� ��ȣ�� �˾Ƴ�����.
	auto mine = m_vpEnvironmentMineItems[item_id];

	mine->SetObtain(false); // ������ O
	mine->SetPosition(GetPosition());// ���� �÷��̾� ��ġ�� ��ġ
	mine->SetInstall(true);
}

void CBlueSuitPlayer::SetEscapePos(XMFLOAT3 pos)
{
	m_fEscapePos = pos;
}


XMFLOAT4X4* CBlueSuitPlayer::GetFlashLigthWorldTransform()
{
	return &m_pFlashlight->m_xmf4x4World;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CZombiePlayer::CZombiePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
	:CPlayer(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext)
{
	m_pCamera->SetFogColor(XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f));
	m_pCamera->SetFogInfo(XMFLOAT4(1.0f, 10.0f, 0.05f, 1.0f));
	m_pCamera->SetUpdateUseRotate(false);

	m_xmf3Scale = XMFLOAT3(1.5f, 1.5f, 1.0f);
	
	UINT ncbElementBytes = ((sizeof(FrameTimeInfo) + 255) & ~255); //256�� ���
	m_pd3dcbTime = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTime->Map(0, NULL, (void**)&m_pcbMappedTime);
	m_d3dTimeCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTime.Get(), ncbElementBytes);

	m_pd3dcbTimeEnd = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTimeEnd->Map(0, NULL, (void**)&m_pcbMappedTimeEnd);
	m_d3dTimeCbvGPUDescriptorHandleEnd = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTimeEnd.Get(), ncbElementBytes);
	

	// ���� �ؽ��� �߰�
	shared_ptr<CTexture> pTexture = make_shared<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
	pTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, (wchar_t*)L"Asset/Textures/elecpatern.dds", RESOURCE_TEXTURE2D, 0);
	m_pElectircaterial = make_shared<CMaterial>(1); // �ؽ�ó�� 1��
	m_pElectircaterial->SetTexture(pTexture, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTexture, 0, 13);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

}

CZombiePlayer::~CZombiePlayer()
{
}

void CZombiePlayer::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);
	LoadBoundingBox(m_voobbOrigin);
	m_pSkinnedAnimationController = make_shared<CZombieAnimationController>(pd3dDevice, pd3dCommandList, 3, pLoadModelInfo);

	m_pBodyObject= FindFrame("Body");
	m_pEyesObject = FindFrame("Eyes");

//	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
//#ifdef _WITH_SOUND_RESOURCE
//	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
//	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
//#else
//
//	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.1f, (void*)_T("Sound/Footstep01.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.9f, (void*)_T("Sound/Footstep02.wav"));
//#endif
//	shared_ptr<CAnimationCallbackHandler> pAnimationCallbackHandler = make_shared<CSoundCallbackHandler>();
//	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);
}

void CZombiePlayer::Update(float fElapsedTime)
{	
	//[0519] ��ų UI�� ���ؼ� �ʿ�
	UpdateSkill(fElapsedTime);

	if (m_bElectricBlend) {
		m_pcbMappedTime->time += fElapsedTime;
		if (m_pcbMappedTime->time > 2.0f) {
			m_bElectricBlend = false;
			m_pcbMappedTime->usePattern = -1.0f;
		}
	}
	else {
		CPlayer::Update(fElapsedTime);
	}

	UpdateAnimation();
}

void CZombiePlayer::UpdateAnimation()
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (::IsZero(fLength))	//�ӷ��� 0�̸� Ʈ�� 0�� �ٽ� true
		{
			if (m_pSkinnedAnimationController->m_nNowState != PlayerState::IDLE)
			{
				if (!m_pSkinnedAnimationController->m_bTransition)
				{
					m_pSkinnedAnimationController->m_bTransition = true;
					m_pSkinnedAnimationController->m_nNextState = PlayerState::IDLE;
				}
			}
		}
		else
		{
			if (m_pSkinnedAnimationController->m_nNowState == PlayerState::IDLE)
			{
				m_pSkinnedAnimationController->m_bTransition = true;
				m_pSkinnedAnimationController->m_nNextState = PlayerState::WALK;
			}


			//sharedobject.AddParticle(CParticleMesh::FOOTPRINT, GetPosition());
		}
	}
}

void CZombiePlayer::UpdateEnding(float fEndingElapsedTime, int nGameState)
{
	switch (nGameState)
	{
	case GAME_STATE::BLUE_SUIT_WIN:
	{
		// �ִ� �ð��� ������ �� �̻� �������� �ʵ��� ����
		float fMaxFogTime = 3.0f; // �Ȱ��� �ִ�ġ�� �����ϴ� �ð� (�� ����)
		if (fEndingElapsedTime > fMaxFogTime)
		{
			fEndingElapsedTime = fMaxFogTime;
		}

		// �ð��� ���� �Ȱ� �е� ��� (0.0���� 1.0���� ����)
		float fFogDensity = 1 - fEndingElapsedTime / fMaxFogTime;

		XMFLOAT4 xmf4EndingFogInfo = XMFLOAT4(1.0f, 0.0f, 0.0f, fFogDensity);
		XMFLOAT4 xmf4EndingFogColor = XMFLOAT4(0.6f - fEndingElapsedTime / 3.0f * 0.5f, 0.6f - fEndingElapsedTime / 3.0f * 0.5f, 0.6f - fEndingElapsedTime / 3.0f * 0.5f, 0.6f - fEndingElapsedTime / 3.0f * 0.5f);
		m_pCamera->SetFogColor(xmf4EndingFogColor);
		m_pCamera->SetFogInfo(xmf4EndingFogInfo);
	}
		break;
	case GAME_STATE::ZOMBIE_WIN:
	{
		// �ִ� �ð��� ������ �� �̻� �������� �ʵ��� ����
		float fMaxFogTime = 3.0f; // �Ȱ��� �ִ�ġ�� �����ϴ� �ð� (�� ����)
		if (fEndingElapsedTime > fMaxFogTime)
		{
			fEndingElapsedTime = fMaxFogTime;
		}

		// �ð��� ���� �Ȱ� �е� ��� (0.0���� 1.0���� ����)
		float fFogDensity = 1 - fEndingElapsedTime / fMaxFogTime;

		XMFLOAT4 xmf4EndingFogInfo = XMFLOAT4(1.0f, 0.0f, 0.0f, fFogDensity);
		XMFLOAT4 xmf4EndingFogColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);
		m_pCamera->SetFogColor(xmf4EndingFogColor);
		m_pCamera->SetFogInfo(xmf4EndingFogInfo);
	}
		break;
	default:
		break;
	}


}

void CZombiePlayer::SetEectricShock()
{ // ���ڿ� �浹�ÿ� ������ �Լ�
	m_pcbMappedTime->time = 0.0f;
	m_bElectricBlend = true;
	m_pcbMappedTime->usePattern = 1.0f;
	if (m_pHitDamageScreenObject) {
		m_pHitDamageScreenObject->SetRender(true);
	}
}

void CZombiePlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_bElectricBlend) {
		m_pElectircaterial->UpdateShaderVariable(pd3dCommandList, nullptr);

		pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);

		CPlayer::Render(pd3dCommandList);

		pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandleEnd);
		return;
	}
	CPlayer::Render(pd3dCommandList);
}

void CZombiePlayer::Animate(float fElapsedTime)
{
	if (m_nClientId == -1)
	{
		//return;
	}

	m_pcbMappedTime->time += fElapsedTime;

	OnUpdateToParent();
	if (!m_bElectricBlend) {
		if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);
	}

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CZombiePlayer::UpdateSkill(float fElapsedTime)
{
	if (!m_bAbleTracking)
	{
		m_fTrackingTime += fElapsedTime;
		if (m_fTrackingTime > TRACKING_COOLTIME)
		{
			m_bAbleTracking = true;
		}
	}

	if (!m_bAbleInterruption)
	{
		m_fInterruptionTime += fElapsedTime;
		if (m_fInterruptionTime > INTERRUPTION_COOLTIME)
		{
			m_bAbleInterruption = true;
		}
	}

	if (!m_bAbleRunning)
	{
		m_fRunningTime += fElapsedTime;
		if (m_fRunningTime > ZOM_RUNNING_COOLTIME)
		{
			m_bAbleRunning = true;
		}
	}
}

void CZombiePlayer::SetTracking(bool bTracking)
{
	m_bTracking = bTracking;
	if (m_bTracking && m_bAbleTracking)
	{
		m_bAbleTracking = false;
		m_fTrackingTime = 0.0f;
	}
}

void CZombiePlayer::SetInterruption(bool bInterruption)
{
	m_bInterruption = bInterruption;
	if (m_bInterruption && m_bAbleInterruption)
	{
		m_bAbleInterruption = false;
		m_fInterruptionTime = 0.0f;
	}
}

void CZombiePlayer::SetRunning(bool bRunning)
{
	m_bRunning = bRunning;
	if (m_bRunning)
	{
		int x = 0;
		x++;
	}
	if (m_bRunning && m_bAbleRunning)
	{
		m_bAbleRunning = false;
		m_fRunningTime = 0.0f;
	}
}


shared_ptr<CCamera> CZombiePlayer::ChangeCamera(DWORD nNewCameraMode, float fElapsedTime)
{
	shared_ptr<CCamera> camera = CPlayer::ChangeCamera(nNewCameraMode, fElapsedTime);
	if (camera->GetMode() != THIRD_PERSON_CAMERA) {		

		//m_pCamera->GenerateProjectionMatrix(0.01f, 100.0f, ASPECT_RATIO, 60.0f);
		int index = dynamic_pointer_cast<CZombieAnimationController>(m_pSkinnedAnimationController)->GetBoneFrameIndex((char*)"EyesSock");
		XMFLOAT3 offset = m_pSkinnedAnimationController->GetBoneFramePositionVector(index);
		//offset.x = 0.0f; offset.z = 0.0f;
		offset.y = offset.y - m_xmf3Position.y;	// [0507]����
		//offset = XMFLOAT3(0.f, 0.f, 0.f);
		camera->SetOffset(offset);
		camera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	}

	auto thisPlayer = shared_from_this();
	dynamic_pointer_cast<CZombieAnimationController>(m_pSkinnedAnimationController)->SetPlayer(dynamic_pointer_cast<CPlayer>(thisPlayer));
	

	return camera;
}

void CZombiePlayer::SetAttackTrail(shared_ptr<Trail> trail)
{
	if (!m_pRightHandTrail) {
		m_pRightHandTrail = trail;

		auto zombieAnimationController = dynamic_pointer_cast<CZombieAnimationController>(m_pSkinnedAnimationController);
		int RightHandThumb4index = zombieAnimationController->GetBoneFrameIndex((char*)"mixamorig:RightHandThumb4");

		// ���� ���� �տ� Ʈ���� �����ǵ����Ѵ�.
		m_pRightHandTrail->SetObject(zombieAnimationController->GetBoneFrameObject(RightHandThumb4index));
		//m_pRightHandTrail->TrailStart();
	}
	else {
		m_pLeftHandTrail = trail;

		auto zombieAnimationController = dynamic_pointer_cast<CZombieAnimationController>(m_pSkinnedAnimationController);
		int LeftHandThumb4index = zombieAnimationController->GetBoneFrameIndex((char*)"mixamorig:LeftHandThumb4");

		// ���� ���� �տ� Ʈ���� �����ǵ����Ѵ�.
		m_pLeftHandTrail->SetObject(zombieAnimationController->GetBoneFrameObject(LeftHandThumb4index));
		//m_pLeftHandTrail->TrailStart();
	}
}

void CZombiePlayer::SetGameStart()
{
	CPlayer::SetGameStart();
}

void CZombiePlayer::RenderTextUI(ComPtr<ID2D1DeviceContext2>& d2dDeviceContext, ComPtr<IDWriteTextFormat>& textFormat, ComPtr<ID2D1SolidColorBrush>& brush)
{
	wchar_t text[128]; // ��ȯ�� �����ڵ� ���ڿ��� ������ ����
	
	// �ε� �Ҽ��� ���� ���ڿ��� ��ȯ �� �����ڵ� ���ڿ��� ����
	if (m_bGameStartWait) {
		static int iPrevCount = 10;
		// ī��Ʈ ���� ����
		static float opacity = 1.0f;

		m_fGameStartCount -= gGameTimer.GetTimeElapsed();
		opacity -= gGameTimer.GetTimeElapsed();

		if (m_fGameStartCount <= 0.1f) {
			m_bGameStartWait = false;

			// �⺻���� �ǵ���.
			m_bInterruption = false;
			m_pCamera->SetFogColor(XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f));
			m_pCamera->SetFogInfo(XMFLOAT4(1.0f, 10.0f, 0.05f, 1.0f));
		}
		int iCeilGameStartCount = ceil(m_fGameStartCount);
		if (iPrevCount > iCeilGameStartCount) {
			iPrevCount = iCeilGameStartCount;
			opacity = 1.0f;
		}

		int len = swprintf(text, 20, L"%d", iCeilGameStartCount);
		text[len + 1] = '\0';

		// ȭ�� �߾ӿ� ���� ������
		POINT windowSize = CGameFramework::GetClientWindowSize();
		//������ ��ǥ ����.
		D2D1_RECT_F textRect = D2D1::RectF(0.f, 0.f, (float)windowSize.x, (float)windowSize.y);
		auto color = brush->GetColor();
		brush->SetColor(D2D1::ColorF(D2D1::ColorF::Tomato));
		brush->SetOpacity(opacity);

		d2dDeviceContext->DrawText(
			text,
			/*_countof(text)*/len,
			CGameFramework::m_idwGameCountTextFormat.Get(),
			&textRect,
			brush.Get()
		);
		brush->SetOpacity(1.0f);

		textRect = D2D1::RectF(0.f, 0.f, (float)windowSize.x, (float)windowSize.y / 2);
		len = swprintf(text, 20, L"��� �����ڸ� �����϶�");
		text[len + 1] = '\0';
		brush->SetColor(D2D1::ColorF(D2D1::ColorF::DarkGray));
		d2dDeviceContext->DrawText(
			text,
			/*_countof(text)*/len,
			CGameFramework::m_idwSpeakerTextFormat.Get(),
			&textRect,
			brush.Get()
		);
		brush->SetColor(color);

		// �þ� ���� �۾�
		if (m_fGameStartCount > 2.5f) {
			m_bInterruption = true;
		}
		else {
			m_bInterruption = false;
		}

		float fogColorValue = 0.1f;

		if (m_bInterruption)
		{
			m_fInterruption = 2.0f;
		}
		else
		{
			m_fInterruption -= gGameTimer.GetTimeElapsed();

			fogColorValue += gGameTimer.GetTimeElapsed();

			if (m_fInterruption <= 0.0f)
			{
				m_fInterruption = 0.0f;
			}
			fogColorValue = (1.0f - m_fInterruption / 2.0f) * 0.5f;
			if (fogColorValue < 0.1f) {
				fogColorValue = 0.1f;
			}
		}
		m_pCamera->SetFogColor(XMFLOAT4(fogColorValue, fogColorValue, fogColorValue, fogColorValue));
		m_pCamera->SetFogInfo(XMFLOAT4(1.0f, 10.0f, 0.05f + m_fInterruption / 8, 1.0f));
	}
}

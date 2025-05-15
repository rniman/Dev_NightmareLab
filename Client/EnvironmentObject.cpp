#include "stdafx.h"
#include "Player.h"
#include "EnvironmentObject.h"

CItemObject::CItemObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CGameObject(pd3dDevice, pd3dCommandList)
{
	SetStatic(false);
}

void CItemObject::Render(ID3D12GraphicsCommandList* pd3dCommandList) 
{
	if (m_bObtained) // �׸��� �ʴ´�
	{
		return;
	}

	CGameObject::Render(pd3dCommandList);
}

void CItemObject::Animate(float fElapsedTime)
{
	if (m_bObtained)
	{
		return;
	}

	CGameObject::Animate(fElapsedTime);

	UpdateTransform(NULL);
}

/// <CGameObject - CItemObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CEnvironmentObject>

CEnvironmentObject::CEnvironmentObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 1;
	m_bStatic = true;
}

/// <CGameObject - CEnvironmentObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CDrawerObject::CDrawerObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh, const shared_ptr<CGameObject>& pGameObject)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 2;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Forward = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Forward = Vector3::TransformNormal(m_xmf3Forward, mtxWorld);

	m_pInstanceObject = pGameObject;
}

CDrawerObject::~CDrawerObject()
{
}

void CDrawerObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	shared_ptr<CGameObject> pGameObject = m_pInstanceObject.lock();
	
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (pGameObject->GetMesh())
	{
		for (int subMeshIndex = 0; subMeshIndex < 1; subMeshIndex++)
		{
			shared_ptr<CInstanceStandardMesh> pInstanceMesh = dynamic_pointer_cast<CInstanceStandardMesh>(pGameObject->GetMesh());
			pInstanceMesh->GetInstanceTransformMatrix()[0] = Matrix4x4::Transpose(m_xmf4x4World);
			UINT8* pBufferDataBegin = NULL;
			pInstanceMesh->GetInstanceTransformMatrixBuffer()->Map(0, NULL, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pInstanceMesh->GetInstanceTransformMatrix(), sizeof(XMFLOAT4X4) * 1);
			pInstanceMesh->GetInstanceTransformMatrixBuffer()->Unmap(0, NULL);

			D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[6] = { pInstanceMesh->GetVertexBufferView(),  pInstanceMesh->GetUV0BufferView(),  pInstanceMesh->GetNormalBufferView(),
				 pInstanceMesh->GetTangentBufferView(), pInstanceMesh->GetBiTangentBufferView(), pInstanceMesh->GetInstanceTransformMatrixBufferView() };
			pd3dCommandList->IASetVertexBuffers(pInstanceMesh->GetSlot(), 6, pVertexBufferViews);

			pd3dCommandList->IASetPrimitiveTopology(pInstanceMesh->GetPrimitiveTopology());

			if ((pInstanceMesh->GetNumOfSubMesh() > 0) && (subMeshIndex < pInstanceMesh->GetNumOfSubMesh()))
			{
				D3D12_INDEX_BUFFER_VIEW dSubSetIndexBufferViews = pInstanceMesh->GetIndexBufferView(subMeshIndex);
				pd3dCommandList->IASetIndexBuffer(&dSubSetIndexBufferViews);
				pd3dCommandList->DrawIndexedInstanced(pInstanceMesh->GetNumOfSubSetIndices(subMeshIndex), 1, 0, 0, 0);
			}
		}
	}
}

void CDrawerObject::Animate(float fElapsedTime)
{
	CGameObject::Animate(fElapsedTime);
}

void CDrawerObject::UpdatePicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_bAnimate = true;
	}
	else
	{
		m_bOpened = true;
		m_bAnimate = true;
	}
}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CDoorObject>

CDoorObject::CDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh, const shared_ptr<CGameObject>& pGameObject)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 2;
	m_pInstanceObject = pGameObject;
	m_nInstanceNumber = dynamic_pointer_cast<CInstanceObject>(pGameObject)->GetInstanceNumber();
}

CDoorObject::~CDoorObject()
{

}

void CDoorObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	shared_ptr<CGameObject> pGameObject = m_pInstanceObject.lock();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (pGameObject->GetMesh())
	{
		for (int subMeshIndex = 0; subMeshIndex < 1; subMeshIndex++)
		{
			shared_ptr<CInstanceStandardMesh> pInstanceMesh = dynamic_pointer_cast<CInstanceStandardMesh>(pGameObject->GetMesh());
			//pInstanceMesh->GetInstanceTransformMatrix()[m_nInstanceNumber] = Matrix4x4::Transpose(m_xmf4x4World);
			//UINT8* pBufferDataBegin = NULL;
			//pInstanceMesh->GetInstanceTransformMatrixBuffer()->Map(0, NULL, (void**)&pBufferDataBegin);
			//memcpy(pBufferDataBegin, pInstanceMesh->GetInstanceTransformMatrix(), sizeof(XMFLOAT4X4) * 1);
			//pInstanceMesh->GetInstanceTransformMatrixBuffer()->Unmap(0, NULL);

			D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[6] = { pInstanceMesh->GetVertexBufferView(),  pInstanceMesh->GetUV0BufferView(),  pInstanceMesh->GetNormalBufferView(),
				 pInstanceMesh->GetTangentBufferView(), pInstanceMesh->GetBiTangentBufferView(), pInstanceMesh->GetInstanceTransformMatrixBufferView() };
			pd3dCommandList->IASetVertexBuffers(pInstanceMesh->GetSlot(), 6, pVertexBufferViews);

			pd3dCommandList->IASetPrimitiveTopology(pInstanceMesh->GetPrimitiveTopology());

			if ((pInstanceMesh->GetNumOfSubMesh() > 0) && (subMeshIndex < pInstanceMesh->GetNumOfSubMesh()))
			{
				D3D12_INDEX_BUFFER_VIEW dSubSetIndexBufferViews = pInstanceMesh->GetIndexBufferView(subMeshIndex);
				pd3dCommandList->IASetIndexBuffer(&dSubSetIndexBufferViews);
				pd3dCommandList->DrawIndexedInstanced(pInstanceMesh->GetNumOfSubSetIndices(subMeshIndex), 1, 0, 0, m_nInstanceNumber);
			}
		}
	}
}

void CDoorObject::Animate(float fElapsedTime)
{
	if( (m_bOpened && m_fRotationAngle < m_fDoorAngle) || (!m_bOpened && m_fRotationAngle > m_fDoorAngle) )
	{
		m_fRotationAngle += m_bOpened ? fElapsedTime * 60.0f : -fElapsedTime * 60.0f;
		float fRotationAngle = m_bOpened ? XMConvertToRadians(fElapsedTime * 60.0f) : -XMConvertToRadians(fElapsedTime * 60.0f);
		if ((m_bOpened && m_fRotationAngle > m_fDoorAngle) || (!m_bOpened && m_fRotationAngle < m_fDoorAngle))
		{
			m_fRotationAngle = m_fDoorAngle;
		}
		else
		{
			//Z-up ����
			XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), XMConvertToRadians(0.0f), fRotationAngle);
			m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	
			// Instancing ��ü�� world��ĸ� �������ָ��
			//UpdateTransform(NULL);
		}
	}

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fElapsedTime, this);

	AnimateOOBB();

	if (m_pSibling) m_pSibling->Animate(fElapsedTime);
	if (m_pChild) m_pChild->Animate(fElapsedTime);
}

void CDoorObject::UpdatePicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_fDoorAngle = 0.0f;
	}
	else
	{
		m_bOpened = true;
		m_fDoorAngle = 150.0f;
	}
}

/// <CGameObject - CDrawerObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CElevatorDoorObject>

CElevatorDoorObject::CElevatorDoorObject(char* pstrFrameName, XMFLOAT4X4& xmf4x4World, CMesh* pMesh, const shared_ptr<CGameObject>& pGameObject)
	: CGameObject(pstrFrameName, xmf4x4World, pMesh)
{
	m_nCollisionType = 2;

	m_xmf3OriginPosition = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);
	m_xmf3Right = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf4x4World);
	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxWorld);

	m_pInstanceObject = pGameObject;
}

void CElevatorDoorObject::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	shared_ptr<CGameObject> pGameObject = m_pInstanceObject.lock();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

	if (pGameObject->GetMesh())
	{
		for (int subMeshIndex = 0; subMeshIndex < 1; subMeshIndex++)
		{
			shared_ptr<CInstanceStandardMesh> pInstanceMesh = dynamic_pointer_cast<CInstanceStandardMesh>(pGameObject->GetMesh());
			pInstanceMesh->GetInstanceTransformMatrix()[0] = Matrix4x4::Transpose(m_xmf4x4World);
			UINT8* pBufferDataBegin = NULL;
			pInstanceMesh->GetInstanceTransformMatrixBuffer()->Map(0, NULL, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pInstanceMesh->GetInstanceTransformMatrix(), sizeof(XMFLOAT4X4) * 1);
			pInstanceMesh->GetInstanceTransformMatrixBuffer()->Unmap(0, NULL);

			D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[6] = { pInstanceMesh->GetVertexBufferView(),  pInstanceMesh->GetUV0BufferView(),  pInstanceMesh->GetNormalBufferView(),
				 pInstanceMesh->GetTangentBufferView(), pInstanceMesh->GetBiTangentBufferView(), pInstanceMesh->GetInstanceTransformMatrixBufferView()};
			pd3dCommandList->IASetVertexBuffers(pInstanceMesh->GetSlot(), 6, pVertexBufferViews);

			pd3dCommandList->IASetPrimitiveTopology(pInstanceMesh->GetPrimitiveTopology());

			if ((pInstanceMesh->GetNumOfSubMesh() > 0) && (subMeshIndex < pInstanceMesh->GetNumOfSubMesh()))
			{
				D3D12_INDEX_BUFFER_VIEW dSubSetIndexBufferViews = pInstanceMesh->GetIndexBufferView(subMeshIndex);
				pd3dCommandList->IASetIndexBuffer(&dSubSetIndexBufferViews);
				pd3dCommandList->DrawIndexedInstanced(pInstanceMesh->GetNumOfSubSetIndices(subMeshIndex), 1, 0, 0, 0);
			}
		}
	}
}

void CElevatorDoorObject::Animate(float fElapsedTime)
{
	XMFLOAT3 xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	float fDistance = Vector3::Distance(xmf3Position, m_xmf3OriginPosition);

	if (m_bAnimate)
	{
		if (m_bOpened)
		{
			if (fDistance < 3.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Right, fElapsedTime * 2.0f);
				Move(xmf3Offset);
			}
			else
			{
				m_bAnimate = false;
			}
		}
		else
		{
			if (fDistance >= 0.0f)
			{
				XMFLOAT3 xmf3Offset = Vector3::ScalarProduct(m_xmf3Right, -fElapsedTime * 2.0f);
				Move(xmf3Offset);
				xmf3Position = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
				XMFLOAT3 xmf3ToPosition = Vector3::Subtract(xmf3Position, m_xmf3OriginPosition);
				if (Vector3::DotProduct(m_xmf3Right, xmf3ToPosition) < 0.0f)
				{
					m_xmf4x4World._41 = m_xmf3OriginPosition.x;
					m_xmf4x4World._42 = m_xmf3OriginPosition.y;
					m_xmf4x4World._43 = m_xmf3OriginPosition.z;
					m_bAnimate = false;
				}
			}
		}
	}

	AnimateOOBB();
}

void CElevatorDoorObject::UpdatePicking()
{
	if (m_bOpened)
	{
		m_bOpened = false;
		m_bAnimate = true;
	}
	else
	{
		m_bOpened = true;
		m_bAnimate = true;
	}
}


/// <CGameObject - CElevatorDoorObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CItemObject - CTeleportObject>

CTeleportObject::CTeleportObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CTeleportObject::~CTeleportObject() 
{
}

void CTeleportObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);
	
	LoadBoundingBox(m_voobbOrigin);
}

void CTeleportObject::Animate(float fElapsedTime)
{
	CItemObject::Animate(fElapsedTime);

	UpdateTransform(NULL);
}


void CTeleportObject::UpdatePicking()
{
	m_bObtained = true;
}

void CTeleportObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	pBlueSuitPlayer->Teleport();
	m_bObtained = false;
}

/// <CGameObject - CTeleportObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CMineObject>

CMineObject::CMineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CMineObject::~CMineObject()
{
}

void CMineObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);
	//�޽�, ����(�ؽ���), 

	LoadBoundingBox(m_voobbOrigin);
}



void CMineObject::Animate(float fElapsedTime)
{
	CItemObject::Animate(fElapsedTime);

	CollideZombie();

	UpdateTransform(NULL);
}

void CMineObject::UpdatePicking()
{
}

void CMineObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
}

void CMineObject::CollideZombie()
{
	if (m_bCollide)
	{
		SetObtain(true);
		m_bCollide = false;
		XMFLOAT3 pos = GetPosition();
		pos.y += 1.0f;
		m_pExplosionObject->SetPosition(pos);
		m_pExplosionObject->m_fLocalTime = 0.0f; // �ִϸ��̼� ����
	}
}

/// <CGameObject - CMineObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CFuseObject>

CFuseObject::CFuseObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CFuseObject::~CFuseObject()
{
}

void CFuseObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);

	LoadBoundingBox(m_voobbOrigin);
}

void CFuseObject::Animate(float fElapsedTime)
{
	CItemObject::Animate(fElapsedTime);

	UpdateTransform(NULL);
}

void CFuseObject::UpdatePicking()
{
	m_bObtained = true;
	m_bCollision = false;
}

void CFuseObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(pGameObject);
	if (!pBlueSuitPlayer)
	{
		return;
	}
	m_bObtained = false;
	m_bCollision = true;
}

/// <CGameObject - CFuseObject>
////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// <CGameObject - CRadarObject>

CRadarObject::CRadarObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
	: CItemObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature)
{
	m_nCollisionType = 2;
}

CRadarObject::~CRadarObject()
{
}

void CRadarObject::LoadModelAndAnimation(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const shared_ptr<CLoadedModelInfo>& pLoadModelInfo)
{
	SetChild(pLoadModelInfo->m_pModelRootObject, true);

	LoadBoundingBox(m_voobbOrigin);
}

void CRadarObject::Animate(float fElapsedTime)
{
	CItemObject::Animate(fElapsedTime);

	UpdateTransform(NULL);
}

void CRadarObject::UpdatePicking()
{
}

void CRadarObject::UpdateUsing(const shared_ptr<CGameObject>& pGameObject)
{
	
}


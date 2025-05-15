#include "stdafx.h"
#include "TCPServer.h"
#include "ServerCollision.h"
#include "ServerObject.h"
#include "ServerPlayer.h"
#include "ServerEnvironmentObject.h"

int CServerCollisionManager::m_nCollisionObject = 0;

void CServerCollisionManager::CreateCollision(int nHeight, int nWidth, int nDepth)
{
	m_nHeight = nHeight;
	m_nWidth = nWidth;
	m_nDepth = nDepth;

	m_collisionGridGameObjects.resize(m_nHeight);
	for (int i = 0; i < m_nHeight; ++i)
	{
		m_collisionGridGameObjects[i].resize(m_nWidth);
		for (int j = 0; j < m_nWidth; ++j)
		{
			m_collisionGridGameObjects[i][j].resize(m_nDepth);
		}
	}
	m_vpCollisionObject.reserve(1000);
}

void CServerCollisionManager::AddCollisionObject(const shared_ptr<CServerGameObject>& pGameObject)
{
	XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
	XMFLOAT3 xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);

	int nWidth = static_cast<int>((xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	int nHeight = static_cast<int>((xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	int nDepth = static_cast<int>((xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);

	m_collisionGridGameObjects[nHeight][nWidth][nDepth].push_back(pGameObject);
	pGameObject->SetSpaceObject(nHeight, nWidth, nDepth);
	/*pGameObject->m_nFloor = nHeight;
	pGameObject->m_nWidth = nWidth;
	pGameObject->m_nDepth = nDepth;*/

	pGameObject->SetCollisionNum(m_nCollisionObject++);
	m_vpCollisionObject.emplace_back(pGameObject);
}

void CServerCollisionManager::ReplaceCollisionObject(const shared_ptr<CServerGameObject>& pGameObject)
{
	//printf("%d %d %d\t", pGameObject->GetFloor(), pGameObject->GetWidth(), pGameObject->GetDepth());
	XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
	XMFLOAT3 xmf3Position = XMFLOAT3(xmf4x4World._41, xmf4x4World._42, xmf4x4World._43);

	int nWidth = static_cast<int>((xmf3Position.x - GRID_START_X) / SPACE_SIZE_XZ);
	int nHeight = static_cast<int>((xmf3Position.y - GRID_START_Y) / SPACE_SIZE_Y);
	int nDepth = static_cast<int>((xmf3Position.z - GRID_START_Z) / SPACE_SIZE_XZ);

	m_collisionGridGameObjects[nHeight][nWidth][nDepth].push_back(pGameObject);
	pGameObject->SetSpaceObject(nHeight, nWidth, nDepth);

	//printf("%d %d %d\n", pGameObject->GetFloor(), pGameObject->GetWidth(), pGameObject->GetDepth());

	m_vpOutSpaceObject.push_back(pGameObject);
}

void CServerCollisionManager::AddCollisionPlayer(const shared_ptr<CServerPlayer>& pPlayer, int nIndex)
{
	m_apPlayer[nIndex] = pPlayer;
}

vpObjects_t& CServerCollisionManager::GetSpaceGameObjects(int nHeight, int nWidth, int nDepth)
{
	return m_collisionGridGameObjects[nHeight][nWidth][nDepth];
}

void CServerCollisionManager::Update(float fElapsedTime)
{
	for (auto& pGameObject : m_vpCollisionObject)
	{
		if (pGameObject)
		{
			if (pGameObject->IsStatic())
			{
				continue;
			}

			auto collisionMgr = shared_from_this();
			pGameObject->Update(fElapsedTime, collisionMgr);
		}
	}
}

void CServerCollisionManager::Collide(float fElapsedTime, const shared_ptr<CServerPlayer>& pPlayer)
{
	if (!pPlayer->IsAlive()) // ����
	{
		return;
	}

	BoundingSphere aabbPlayer;
	/*BoundingBox aabbPlayer;
	aabbPlayer.Center = pPlayer->GetOOBB(0).Center;
	aabbPlayer.Extents = pPlayer->GetOOBB(0).Extents;
	XMVECTOR xmvTranslation = XMVectorSet(pPlayer->GetPosition().x, pPlayer->GetPosition().y, pPlayer->GetPosition().z, 1.0f);
	aabbPlayer.Transform(aabbPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);*/
	
	//aabbPlayer = pPlayer->GetOOBB(0);
	aabbPlayer.Center = pPlayer->GetOOBB(0).Center;
	aabbPlayer.Radius = pPlayer->GetOOBB(0).Extents.z;
	XMFLOAT4X4 xmf4x4World = pPlayer->GetWorldMatrix();
	aabbPlayer.Transform(aabbPlayer, XMLoadFloat4x4(&xmf4x4World));
	//XMStoreFloat4(&aabbPlayer.Orientation, XMQuaternionNormalize(XMLoadFloat4(&aabbPlayer.Orientation)));

	shared_ptr<CServerZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CServerZombiePlayer>(pPlayer);
	// �÷��̾� �浹�˻�
	for (const auto& pwOtherPlayer : m_apPlayer)
	{
		shared_ptr<CServerPlayer> pOtherPlayer = pwOtherPlayer.lock();
		if (!pOtherPlayer || pOtherPlayer->GetPlayerId() == -1)
		{
			continue;
		}

		if (!pOtherPlayer->IsAlive() || !pOtherPlayer->IsCollision())	// �浹 �˻� ����� �ƴ�(EX: ����)
		{
			continue;
		}

		if (pPlayer->GetPlayerId() == pOtherPlayer->GetPlayerId())
		{
			continue;
		}

		BoundingSphere aabbOtherPlayer;
		aabbOtherPlayer.Center = pOtherPlayer->GetOOBB(0).Center;
		aabbOtherPlayer.Radius = pOtherPlayer->GetOOBB(0).Extents.z;
		//aabbOtherPlayer.Extents = pOtherPlayer->GetOOBB(0).Extents;
		//XMFLOAT4 xmf4OtherPosition = XMFLOAT4(pOtherPlayer->GetPosition().x, pOtherPlayer->GetPosition().y, pOtherPlayer->GetPosition().z, 1.0f);
		//XMVECTOR xmvTranslation = XMLoadFloat4(&xmf4OtherPosition);
		//aabbOtherPlayer.Transform(aabbOtherPlayer, 1.0f, XMQuaternionIdentity(), xmvTranslation);
		XMFLOAT4X4 xmf4x4OtherWorld = pOtherPlayer->GetWorldMatrix();
		aabbOtherPlayer.Transform(aabbOtherPlayer, XMLoadFloat4x4(&xmf4x4OtherWorld));


		if (aabbOtherPlayer.Intersects(aabbPlayer))
		{
			//pPlayer->Collide(shared_from_this(), fElapsedTime, pOtherPlayer);

			pPlayer->CollideWithPlayer(shared_from_this(), fElapsedTime, pOtherPlayer);
		}

		if (pOtherPlayer->IsInvincibility())
		{
			continue;
		}

		if (pZombiePlayer)
		{
			if (pOtherPlayer->IsAlive() && pZombiePlayer->IsAttack())
			{
				pZombiePlayer->CheckAttack(pOtherPlayer, aabbOtherPlayer);
			}
		}
	}

	for (int i = pPlayer->GetWidth() - 2; i <= pPlayer->GetWidth() + 2; ++i)
	{
		for (int j = pPlayer->GetDepth() - 2; j <= pPlayer->GetDepth() + 2; ++j)
		{
			if (i < 0 || i >= GetWidth() || j < 0 || j >= GetDepth())
			{
				continue;
			}

			for (const auto& pGameObject : GetSpaceGameObjects(pPlayer->GetFloor(), i, j))	// �÷��̾��� �׸��带 ��Ÿ������
			{
				if (!pGameObject || !pGameObject->IsCollision())
				{
					continue;
				}

				if (pGameObject->GetCollisionType() == StairTrigger)
				{
					CheckStairTrigger(pGameObject, aabbPlayer, pPlayer);
					continue;
				}
				else if (pGameObject->GetCollisionType() != Standard)
				{
					BoundingBox a;
					a.Center = aabbPlayer.Center;
					a.Extents = pPlayer->GetOOBB(0).Extents;
					CollideWithMine(pGameObject, pPlayer, a);
					continue;
				}

				for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
				{
					BoundingOrientedBox oobb;
					XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
					oobbOrigin.Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
					XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

					if (!oobb.Intersects(aabbPlayer))
					{
						continue;
					}

					pPlayer->Collide(shared_from_this(), fElapsedTime, pGameObject);
					break;
				}
			}
		}
	}
	
	if (pPlayer->IsStair())
	{
		PlayerInStair(pPlayer);
	}
}

void CServerCollisionManager::PlayerInStair(const std::shared_ptr<CServerPlayer>& pPlayer)
{
	XMFLOAT3 xmf3StairPosition = Plane::CalculatePointY(pPlayer->GetStairPlane(), pPlayer->GetPosition());
	if (xmf3StairPosition.y - EPSILON <= pPlayer->GetStairMin())
	{
		xmf3StairPosition.y = pPlayer->GetStairMin();
		pPlayer->SetStair(false);
	}
	else if (xmf3StairPosition.y + EPSILON >= pPlayer->GetStairMax())
	{
		xmf3StairPosition.y = pPlayer->GetStairMax();
		pPlayer->SetStair(false);
	}
	pPlayer->SetPlayerPosition(xmf3StairPosition);
}

void CServerCollisionManager::CheckStairTrigger(const std::shared_ptr<CServerGameObject>& pGameObject, DirectX::BoundingSphere& aabbPlayer, const std::shared_ptr<CServerPlayer>& pPlayer)
{
	BoundingOrientedBox oobb;
	XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
	pGameObject->GetOOBB(0).Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
	XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));
	if (oobb.Intersects(aabbPlayer))
	{
		pPlayer->SetStair(true);
		shared_ptr<CServerStairTriggerObject> pStairObject = dynamic_pointer_cast<CServerStairTriggerObject>(pGameObject);
		if (pStairObject)
		{
			if (pStairObject->GetOffsetY() < 0.0f)
			{
				pPlayer->SetStairY(pStairObject->GetY() - 0.2f, pStairObject->GetY() - 0.2f - 4.5f);
			}
			else
			{
				pPlayer->SetStairY(pStairObject->GetY() - 0.2f + 4.5f, pStairObject->GetY() - 0.2f);
			}
			pPlayer->SetStairPlane(pStairObject->GetStairPlane());
		}
	}
}

void CServerCollisionManager::CollideWithPlayer(float fElapsedTime)
{
}

void CServerCollisionManager::CollideWithMine(shared_ptr<CServerGameObject> pGameObject, shared_ptr<CServerGameObject> pPlayer, BoundingBox& aabbPlayer)
{
	if (pGameObject->GetCollisionType() != Picking) {
		return;
	}

	auto zombiePlayer = dynamic_pointer_cast<CServerZombiePlayer>(pPlayer);
	if (!zombiePlayer) {
		return;
	}
	//���� �浹 aabbPlayer
	auto mine = dynamic_pointer_cast<CServerMineObject>(pGameObject);
	if (!mine) {
		return;
	}

	if (zombiePlayer->GetNoStopTime() > 0.0f || !mine->IsInstall()) {
		return;
	}
	for (const auto& oobbOrigin : pGameObject->GetVectorOOBB())
	{
		BoundingOrientedBox oobb;
		XMFLOAT4X4 xmf4x4World = pGameObject->GetWorldMatrix();
		oobbOrigin.Transform(oobb, XMLoadFloat4x4(&xmf4x4World));
		XMStoreFloat4(&oobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&oobb.Orientation)));

		if (!oobb.Intersects(aabbPlayer))
		{
			return;
		}
		//std::cout << "���� �浹" << std::endl;
		mine->SetObtain(true);
		mine->SetExplosionTime(2.0f);
		zombiePlayer->CollisionMine(mine->GetCollisionNum());
	}
}


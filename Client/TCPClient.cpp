#include "stdafx.h"
#include "TCPClient.h"
#include "GameFramework.h"
#include "Player.h"
#include "SharedObject.h"
#include "Sound.h"

CTcpClient::CTcpClient()
{
}

CTcpClient::~CTcpClient()
{
}

bool CTcpClient::CreateSocket(HWND hWnd, TCHAR* pszIPAddress)
{
	int nRetval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		err_quit("WSAStartup");
		return false;
	}

	// ���� ����
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	m_sock = s;
	//m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		err_quit("socket()");
		return false;	
	}

	char pIPAddress[20];
	ConvertLPWSTRToChar(pszIPAddress, pIPAddress, 20);

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	inet_pton(AF_INET, pIPAddress, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	//nRetval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	nRetval = connect(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
 	if (nRetval == SOCKET_ERROR) 
	{
		err_display("connect()");
		return false;
	}

	//nRetval = WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE�� �߻��Ұ��̴�.
	nRetval = WSAAsyncSelect(s, hWnd, WM_SOCKET, FD_CLOSE | FD_READ | FD_WRITE);	// FD_WRITE�� �߻��Ұ��̴�.
	if (nRetval == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

void CTcpClient::OnDestroy()
{

}


XMFLOAT3 CTcpClient::GetPostion(int id)
{
	XMFLOAT3 position = { 0.0f,0.0f, 0.0f };

	return position;
}

std::array<CS_CLIENTS_INFO, 5>& CTcpClient::GetArrayClientsInfo()
{
	return m_aClientInfo;
}

void CTcpClient::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:	// ������ �����͸� ���� �غ� �Ǿ���.
		OnProcessingReadMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_WRITE:	// ������ �����͸� ������ �غ� �Ǿ���.
		OnProcessingWriteMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case FD_CLOSE:
		closesocket(wParam);
		WSACleanup();	
		break;
	default:
		break;
	}
}

void CTcpClient::OnProcessingReadMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	static INT8 nHead;
	int nRetval = 1;
	size_t nBufferSize;

	if(!m_bRecvHead)
	{
		nBufferSize = sizeof(INT8);
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			if (nRetval == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				m_bRecvHead = false;
				nHead = -1;
				memset(m_pCurrentBuffer, 0, BUFSIZE);
			}
			return;
		}
		m_bRecvHead = true;
		memcpy(&nHead, m_pCurrentBuffer, sizeof(INT8));
		memset(m_pCurrentBuffer, 0, BUFSIZE);
	}

	switch (nHead)
	{
	case HEAD_GAME_START:
		PostMessage(hWnd, WM_START_GAME, 0, 0);
		m_socketState = SOCKET_STATE::SEND_KEY_BUFFER;
		break;
	case HEAD_CHANGE_SLOT:
	{	//Ŭ�� id �ٲٱ�, 
		nBufferSize = sizeof(INT8) + sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		INT8 nPrevMainClientId = m_nMainClientId;
		memcpy(&m_nMainClientId, m_pCurrentBuffer, sizeof(INT8));
		if (nRetval != 0)
		{
			break;
		}
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(INT8), sizeof(m_aClientInfo));
		for (int i = 0; i < MAX_CLIENT; ++i)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			}
		}

		if (nPrevMainClientId != m_nMainClientId)
		{
			PostMessage(hWnd, WM_CHANGE_SLOT, 1, 0);
		}
	}
		break;
	case HEAD_INIT:
		//nBufferSize = sizeof(INT8) * 2;
		nBufferSize = sizeof(INT8) * 2 + sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nMainClientId, m_pCurrentBuffer, sizeof(INT8));
		memcpy(&m_nClient, m_pCurrentBuffer + sizeof(INT8), sizeof(INT8));
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(INT8) * 2, sizeof(m_aClientInfo));

		break;
	case HEAD_UPDATE_DATA:
	{
		nBufferSize = sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}
		
		memcpy(m_aClientInfo.data(), m_pCurrentBuffer, sizeof(m_aClientInfo));

		UpdateDataFromServer();
	}
	break;
	case HEAD_NUM_OF_CLIENT:
		nBufferSize = sizeof(INT8) + sizeof(m_aClientInfo);
		RecvNum++;
		nRetval = RecvData(wParam, nBufferSize);
		if (nRetval != 0)
		{
			break;
		}

		memcpy(&m_nClient, m_pCurrentBuffer, sizeof(INT8));
		memcpy(&m_aClientInfo, m_pCurrentBuffer + sizeof(INT8), sizeof(m_aClientInfo));
		for (int i = 0;i < MAX_CLIENT;++i)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			}
		}
		break;
	case HEAD_BLUE_SUIT_WIN:
		// �޽���?
		PostMessage(hWnd, WM_END_GAME, 0, 0);
		break;
	case HEAD_ZOMBIE_WIN:
		PostMessage(hWnd, WM_END_GAME, 1, 0);
		break;
	case HEAD_OPEN_DRAWER_SOUND:
	{
		SoundManager& soundManager = soundManager.GetInstance();
		soundManager.PlaySoundWithName(sound::OPEN_DRAWER);
	}
		break;
	case HEAD_CLOSE_DRAWER_SOUND:
	{
		SoundManager& soundManager = soundManager.GetInstance();
		soundManager.PlaySoundWithName(sound::CLOSE_DRAWER);
	}
	break;
	case HEAD_OPEN_DOOR_SOUND:
	{
		SoundManager& soundManager = soundManager.GetInstance();
		soundManager.PlaySoundWithName(sound::OPEN_DOOR);
	}
		break;
	case HEAD_CLOSE_DOOR_SOUND:
	{
		SoundManager& soundManager = soundManager.GetInstance();
		soundManager.PlaySoundWithName(sound::CLOSE_DOOR);
	}
	break;
	case HEAD_BLUE_SUIT_DEAD:
	{
		nRetval = RecvData(wParam, sizeof(char)); // 1����Ʈ deaduser_id
		if (nRetval != 0)
		{
			break;
		}
		char deadUser_id;
		memcpy(&deadUser_id, m_pCurrentBuffer, sizeof(char));
		
		SoundManager& soundManager = soundManager.GetInstance();
		soundManager.PlaySoundWithName(sound::DEAD_BLUESUIT);
		soundManager.SetVolume(sound::DEAD_BLUESUIT, m_apPlayers[(int)deadUser_id]->GetPlayerVolume());
	}
	break;
	case SEND_SPACEOUT_OBJECTS: {
		nRetval = RecvData(wParam, sizeof(unsigned short));
		if (nRetval != 0)
		{
			break;
		}
		unsigned short usBufferSize;
		memcpy(&usBufferSize, m_pCurrentBuffer, sizeof(unsigned short));
		int objCount = usBufferSize / sizeof(SC_SPACEOUT_OBJECT);
		vector<SC_SPACEOUT_OBJECT> vSO_obejcts;
		vSO_obejcts.reserve(objCount);
		vSO_obejcts.resize(objCount);

		nRetval = RecvData(wParam, usBufferSize);
		if (nRetval != 0)
		{
			break;
		}
		memcpy(vSO_obejcts.data(), m_pCurrentBuffer, usBufferSize);
		for(const auto& obj : vSO_obejcts){
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(obj.m_iObjectId).lock();
			if (pGameObject)
			{
				pGameObject->m_xmf4x4World = obj.m_xmf4x4World;
				pGameObject->m_xmf4x4ToParent = obj.m_xmf4x4World;
				pGameObject->SetObtain(false);
			}
		}
		break;
	}
	case HEAD_LOADING_COMPLETE: {
		m_bRecvLoadComplete = true;
		break;
	}
	default:
		break;
	}

	if (nRetval != 0)
	{
		/*if (nRetval == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			m_bRecvHead = true;
			memset(m_pCurrentBuffer, 0, BUFSIZE);
		}*/
		return;
	}
	nHead = -1;
	m_bRecvHead = false;
	m_bRecvDelayed = false;
	memset(m_pCurrentBuffer, 0, BUFSIZE);

	return;
}

void CTcpClient::UpdateDataFromServer()
{
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (m_apPlayers[i])
		{
			m_apPlayers[i]->SetAlive(m_aClientInfo[i].m_bAlive);
			m_apPlayers[i]->SetRunning(m_aClientInfo[i].m_bRunning);
			m_apPlayers[i]->SetClientId(m_aClientInfo[i].m_nClientId);
			m_apPlayers[i]->SetPosition(m_aClientInfo[i].m_xmf3Position);
			m_apPlayers[i]->SetVelocity(m_aClientInfo[i].m_xmf3Velocity);
			if (i != m_nMainClientId) {
				m_apPlayers[i]->SetPitch(m_aClientInfo[i].m_animationInfo.pitch);
			}


			if (i != m_nMainClientId)
			{
				m_apPlayers[i]->SetLook(m_aClientInfo[i].m_xmf3Look);
				XMFLOAT3 xmf3Right = XMFLOAT3(0.0f, 1.0f, 0.0f);
				xmf3Right = Vector3::CrossProduct(xmf3Right, m_aClientInfo[i].m_xmf3Look, true);
				m_apPlayers[i]->SetRight(xmf3Right);
				//m_apPlayers[i]->SetRight(m_aClientInfo[i].m_xmf3Right);
			}

			//[0523] ��ŷ ������Ʈ ����(�ܰ��� �۾��� �ʿ�)
			UpdatePickedObject(i);

			// ���� �浹
			int nObjectNum = m_aClientInfo[i].m_playerInfo.m_iMineobjectNum;
			if (nObjectNum >= 0) {
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
				auto mine = dynamic_pointer_cast<CMineObject>(pGameObject);
				if (mine)
				{
					mine->SetCollide(true);
					shared_ptr<CZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayers[i]);
					if (pZombiePlayer)
					{
						pZombiePlayer->SetEectricShock();
					}

					float fVolume = m_apPlayers[i]->GetPlayerVolume();
					SoundManager& soundManager = soundManager.GetInstance();
					if (m_apPlayers[i]->GetPlayerVolume() - EPSILON >= 0.0f) soundManager.PlaySoundWithName(sound::ACTIVE_MINE, fVolume);
				}
			}
		}

		if (i == ZOMBIEPLAYER)
		{
			UpdateZombiePlayer();
		}
		else
		{
			UpdatePlayer(i);
		}

		int nNumOfGameObject = m_aClientInfo[i].m_nNumOfObject;
		for (int j = 0; j < nNumOfGameObject; ++j)
		{
			int nObjectNum = m_aClientInfo[i].m_anObjectNum[j];

			if (nObjectNum <= -1 || nObjectNum >= g_collisionManager.GetNumOfCollisionObject())
			{
				continue;
			}
#ifdef LOADSCENE
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
			if (pGameObject)
			{
				pGameObject->m_xmf4x4World = m_aClientInfo[i].m_axmf4x4World[j];
				pGameObject->m_xmf4x4ToParent = m_aClientInfo[i].m_axmf4x4World[j];
			}
#endif LOADSCENE
		}
	}
}

void CTcpClient::UpdatePickedObject(int i)
{
	if (i == m_nMainClientId)
	{
		if (m_aClientInfo[i].m_nPickedObjectNum == -1)
		{
			m_apPlayers[i]->SetPickedObject(nullptr);
		}
		else
		{
			int nObjectNum = m_aClientInfo[i].m_nPickedObjectNum;
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(nObjectNum).lock();
			if (pGameObject)
			{
				m_apPlayers[i]->SetPickedObject(pGameObject);
			}
		}
	}
}

void CTcpClient::OnProcessingWriteMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	size_t nBufferSize = sizeof(INT8);
	INT8 nHead;
	int nRetval;
	if (m_nMainClientId == -1 || m_bRecvDelayed == true || !m_apPlayers[m_nMainClientId])	// ���� ID�� �Ѱ� ���� ���߰ų� ������ �Ǿ���.
	{
		return;
	}
	
	//������ ���� �� ����
	m_aClientInfo[m_nMainClientId].m_animationInfo.pitch = m_apPlayers[m_nMainClientId]->GetPitch();
	m_aClientInfo[m_nMainClientId].m_playerInfo.m_bRightClick = m_apPlayers[m_nMainClientId]->IsRightClick();
	m_apPlayers[m_nMainClientId]->SetRightClick(false);

	switch (m_socketState)
	{
	case SOCKET_STATE::SEND_GAME_START:
		nHead = 1;
		nRetval = SendData(m_sock, nBufferSize, nHead);

		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
		break;
	case SOCKET_STATE::SEND_CHANGE_SLOT:
		nHead = 2;
		nBufferSize += sizeof(INT8);
		nRetval = SendData(m_sock, nBufferSize, nHead, m_nSelectedSlot);

		m_socketState = SOCKET_STATE::SEND_GAME_START;
		break;
	case SOCKET_STATE::SEND_KEY_BUFFER:
	{
		nHead = 0;
		UCHAR keysBuffer[256];

		UCHAR* pKeysBuffer = CGameFramework::GetKeysBuffer();
		WORD wKeyBuffer = 0;
		UpdateKeyBitMask(pKeysBuffer, wKeyBuffer);
		
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

		nBufferSize += sizeof(WORD);

		//nBufferSize += sizeof(keysBuffer);
		//if (pKeysBuffer != nullptr)
		//{
		//	memcpy(keysBuffer, pKeysBuffer, nBufferSize - sizeof(int));
		//}
		nBufferSize += sizeof(std::chrono::time_point<std::chrono::steady_clock>);
		nBufferSize += sizeof(XMFLOAT4X4);
		nBufferSize += sizeof(XMFLOAT3) * 3;
		nBufferSize += sizeof(CS_ANIMATION_INFO);
		nBufferSize += sizeof(CS_PLAYER_INFO);

		SendNum++;
		// Ű����, ī�޶�Matrix, LOOK,RIGHT ���� �����ֱ�
		if(m_apPlayers[m_nMainClientId]->m_pSkinnedAnimationController->IsAnimation())
		{
			nRetval = SendData(wParam, nBufferSize,
				nHead,
				now,
				wKeyBuffer,
				m_apPlayers[m_nMainClientId]->GetCamera()->GetViewMatrix(),
				m_apPlayers[m_nMainClientId]->GetLook(),
				m_apPlayers[m_nMainClientId]->GetRight(),
				m_apPlayers[m_nMainClientId]->GetUp(),
				m_aClientInfo[m_nMainClientId].m_animationInfo,
				m_aClientInfo[m_nMainClientId].m_playerInfo
			);
		}
		else
		{
			nRetval = SendData(wParam, nBufferSize,
				nHead,
				now,
				wKeyBuffer,
				m_apPlayers[m_nMainClientId]->GetCamera()->GetViewMatrix(),
				m_apPlayers[m_nMainClientId]->GetCamera()->GetLookVector(),
				m_apPlayers[m_nMainClientId]->GetCamera()->GetRightVector(),
				m_apPlayers[m_nMainClientId]->GetCamera()->GetUpVector(),
				m_aClientInfo[m_nMainClientId].m_animationInfo,
				m_aClientInfo[m_nMainClientId].m_playerInfo
			);
		}

		if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
		}
	}
		break;
	default:
		break;
	}
}

template<class... Args>
void CTcpClient::CreateSendDataBuffer(char* pBuffer, Args&&... args)
{
	size_t nOffset = 0;
	((memcpy(pBuffer + nOffset, &args, sizeof(args)), nOffset += sizeof(args)), ...);
}

template<class... Args>
int CTcpClient::SendData(SOCKET socket, size_t nBufferSize, Args&&... args)
{
	int nRetval;
	char* pBuffer = new char[nBufferSize];
	(CreateSendDataBuffer(pBuffer, args...));

	nRetval = send(socket, (char*)pBuffer, nBufferSize, 0);
	delete[] pBuffer;

	if (nRetval == SOCKET_ERROR)
	{
		return -1;
	}
	return 0;
}

int CTcpClient::RecvData(SOCKET socket, size_t nBufferSize)
{
	int nRetval;
	int nRemainRecvByte = nBufferSize - m_nCurrentRecvByte;

	nRetval = recv(socket, (char*)&m_pCurrentBuffer + m_nCurrentRecvByte, nRemainRecvByte, 0);
	if(nRetval > 0) m_nCurrentRecvByte += nRetval;
	if (nRetval == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}
	else if (nRetval == 0)
	{
		return -2;
	}
	else if (m_nCurrentRecvByte < nBufferSize)
	{
		m_bRecvDelayed = true;
		return 1;
	}
	else
	{
		m_nCurrentRecvByte = 0;
		m_bRecvDelayed = false;
		return 0;
	}
}

void CTcpClient::UpdateKeyBitMask(UCHAR* pKeysBuffer, WORD& wKeyBuffer)	// ���� Ű ���۸� ������Ʈ
{
	if (pKeysBuffer['W'] & 0xF0)wKeyBuffer |= KEY_W;
	if (pKeysBuffer['S'] & 0xF0)wKeyBuffer |= KEY_S;
	if (pKeysBuffer['A'] & 0xF0)wKeyBuffer |= KEY_A;
	if (pKeysBuffer['D'] & 0xF0)wKeyBuffer |= KEY_D;
	if (pKeysBuffer['1'] & 0xF0)wKeyBuffer |= KEY_1;
	if (pKeysBuffer['2'] & 0xF0)wKeyBuffer |= KEY_2;
	if (pKeysBuffer['3'] & 0xF0)wKeyBuffer |= KEY_3;
	if (pKeysBuffer['4'] & 0xF0)wKeyBuffer |= KEY_4;
	if (pKeysBuffer['E'] & 0xF0)wKeyBuffer |= KEY_E;
	if (pKeysBuffer[VK_LSHIFT] & 0xF0)wKeyBuffer |= KEY_LSHIFT;
	if (pKeysBuffer[VK_LBUTTON] & 0xF0)wKeyBuffer |= KEY_LBUTTON;
	if (pKeysBuffer[VK_RBUTTON] & 0xF0)wKeyBuffer |= KEY_RBUTTON;
}

void CTcpClient::UpdateZombiePlayer()
{
	shared_ptr<CZombiePlayer> pZombiePlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayers[0]);
	if (!pZombiePlayer)
	{
		return;
	}

	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (m_nMainClientId == ZOMBIEPLAYER)	// ����
		{
			if (m_aClientInfo[0].m_nSlotObjectNum[0] == 1)
			{
				m_apPlayers[i]->SetTracking(true);
			}
			else
			{
				m_apPlayers[i]->SetTracking(false);
			}
		}

		if (m_apPlayers[i]->GetClientId() != m_nMainClientId || i == ZOMBIEPLAYER)
		{
			continue;
		}
		if (m_aClientInfo[0].m_nSlotObjectNum[1] == 1)
		{
			m_apPlayers[i]->SetInterruption(true);
		}
		else
		{
			m_apPlayers[i]->SetInterruption(false);
		}
	}

	// �þ� ����(zombie �÷��̾�)
	if (m_nMainClientId == ZOMBIEPLAYER)
	{
		if (m_aClientInfo[0].m_nSlotObjectNum[1] == 1)
		{
			m_apPlayers[0]->SetInterruption(true);
		}
		else
		{
			m_apPlayers[0]->SetInterruption(false);
		}
	}

	if (m_aClientInfo[0].m_nSlotObjectNum[2] == 1)	// ������ �õ�
	{
		pZombiePlayer->m_pSkinnedAnimationController->SetTrackEnable(2, true);
	}
}

void CTcpClient::UpdatePlayer(int nIndex)
{
	shared_ptr<CBlueSuitPlayer> pBlueSuitPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayers[nIndex]);

	if (!pBlueSuitPlayer) // �����ڰ� �ƴϸ� ���� x
	{
		return;
	}

	if (m_nEscapeDoor == -1) m_nEscapeDoor = m_aClientInfo[nIndex].m_playerInfo.m_iEscapeDoor;
	if (m_nEscapeDoor != -1) {
		shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_nEscapeDoor).lock();
		pBlueSuitPlayer->SetEscapePos(pGameObject->GetPosition());
	}

	pBlueSuitPlayer->SelectItem(m_aClientInfo[nIndex].m_playerInfo.m_selectItem);
	for (int j = 0; j < 3; ++j)
	{
		if (m_aClientInfo[nIndex].m_nSlotObjectNum[j] != -1)
		{
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_aClientInfo[nIndex].m_nSlotObjectNum[j]).lock();
			shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
			if (pItemObject) {
				if (!pItemObject->IsObtained()) {
					// �������� ȹ���� ����
					sharedobject.EnableItemGetParticle(pItemObject);
				}
				pItemObject->SetObtain(true);
				if (nIndex == m_nMainClientId && !pBlueSuitPlayer->IsSlotItemObtain(j))
				{
					SoundManager& soundManager = soundManager.GetInstance();
					soundManager.PlaySoundWithName(sound::GET_ITEM_BLUESUIT);
				}
				pBlueSuitPlayer->SetSlotItem(j, m_aClientInfo[nIndex].m_nSlotObjectNum[j]);
			}
		}
		else // -1�� �޾Ҵµ� �÷��̾ ���� Reference���� -1�� �ƴ� ��츦 �����ؾ���
		{
			if (pBlueSuitPlayer->GetReferenceSlotItemNum(j) != -1)
			{
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(pBlueSuitPlayer->GetReferenceSlotItemNum(j)).lock();
				shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
				if (pItemObject) {
					pItemObject->SetObtain(false);
					pBlueSuitPlayer->SetSlotItemEmpty(j);
				}
			}
		}
	}

	for (int j = 0; j < 3; ++j)
	{
		if (m_aClientInfo[nIndex].m_nFuseObjectNum[j] != -1)
		{
			shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(m_aClientInfo[nIndex].m_nFuseObjectNum[j]).lock();
			shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
			if (pItemObject) {
				if (!pItemObject->IsObtained()) {
					// �������� ȹ���� ����
					sharedobject.EnableItemGetParticle(pItemObject);
				}
				pItemObject->SetObtain(true);
				if (nIndex == m_nMainClientId && !pBlueSuitPlayer->IsFuseObtain(j))
				{
					SoundManager& soundManager = soundManager.GetInstance();
					soundManager.PlaySoundWithName(sound::GET_ITEM_BLUESUIT);
				}
				pBlueSuitPlayer->SetFuseItem(j, m_aClientInfo[nIndex].m_nFuseObjectNum[j]);
			}
		}
		else // -1�� �޾Ҵµ� �÷��̾ ���� Reference���� -1�� �ƴ� ��츦 �����ؾ���
		{
			if (pBlueSuitPlayer->GetReferenceFuseItemNum(j) != -1)
			{
				shared_ptr<CGameObject> pGameObject = g_collisionManager.GetCollisionObjectWithNumber(pBlueSuitPlayer->GetReferenceFuseItemNum(j)).lock();
				shared_ptr<CItemObject> pItemObject = dynamic_pointer_cast<CItemObject>(pGameObject);
				if (pItemObject) {
					pItemObject->SetObtain(false);
					pBlueSuitPlayer->SetFuseItemEmpty(j);
				}
			}
		}
	}

	if (m_aClientInfo[nIndex].m_playerInfo.m_bAttacked) {
		pBlueSuitPlayer->SetHitEvent();
	}

	if (m_aClientInfo[nIndex].m_playerInfo.m_bTeleportItemUse) {
		pBlueSuitPlayer->Teleport();
	}
}

void CTcpClient::LoadCompleteSend()
{
	INT8 nHead = static_cast<INT8>(SOCKET_STATE::SEND_LOADING_COMPLETE);
	size_t nBufferSize = sizeof(INT8);
	int nRetval = SendData(m_sock, nBufferSize, nHead);

	if (nRetval == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
	{
	}
}

void ConvertLPWSTRToChar(LPWSTR lpwstr, char* dest, int destSize)
{
	// WideCharToMultiByte �Լ��� ����Ͽ� LPWSTR�� char*�� ��ȯ
	WideCharToMultiByte(
		CP_UTF8,
		0,                   // ��ȯ �ɼ�
		lpwstr,              // ��ȯ�� �����ڵ� ���ڿ�
		-1,                  // �ڵ����� ���ڿ� ���� ���
		dest,                // ��� ����
		destSize,            // ��� ������ ũ��
		NULL,                // �⺻ ���� ��� �� ��
		NULL                 // �⺻ ���� ��� ���θ� ������ ������ �ּ�
	);
}

void ConvertCharToLPWSTR(const char* pstr, LPWSTR dest, int destSize)
{
	// MultiByteToWideChar �Լ��� ����Ͽ� char*�� LPWSTR�� ��ȯ
	MultiByteToWideChar(
		CP_UTF8,
		0,                   // ��ȯ �ɼ�
		pstr,                 // ��ȯ�� ���ڿ�
		-1,                  // �ڵ����� ���ڿ� ���� ���
		dest,                // ��� ����
		destSize             // ��� ������ ũ��
	);
}


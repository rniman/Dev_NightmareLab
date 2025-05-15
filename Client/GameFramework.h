#pragma once
#include "Timer.h"
#include "Scene.h"
#include "TCPClient.h"

struct CB_FRAMEWORK_INFO
{
	float					m_fCurrentTime;
	float					m_fElapsedTime;
	float					m_fSecondsPerFirework = 1.0f;
	int						m_nFlareParticlesToEmit = -1;
	XMFLOAT3				m_xmf3Gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);
	int						m_nMaxFlareType2Particles = -1;
};

//constexpr size_t SWAPCHAIN_BUFFER_NUM = 2;
//class TextObject;

constexpr UINT WM_CREATE_TCP{ WM_USER + 2 };
constexpr UINT WM_END_GAME{ WM_USER + 3 };
constexpr UINT WM_START_GAME{ WM_USER + 4 };
constexpr UINT BUTTON_CREATE_TCP_ID{ 1 };
constexpr UINT EDIT_INPUT_ADDRESS_ID{ 2 };

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
	void OnDestroyEntryWindow();

	void CreateEntryWindow(HWND hWnd);

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void CreateMainScene();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void AnimateEnding();
	//void ProcessCollide();
	void PreRenderTasks(shared_ptr<CMainScene>& pMainScene);
	void FrameAdvance();
	void LoadingRender();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingCommandMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void OnProcessingEndGameMessage(WPARAM& wParam);

	INT8 GetClientIdFromTcpClient() const { return m_pTcpClient->GetMainClientId(); }

	static UCHAR* GetKeysBuffer();
	static int GetMainClientId() { return m_nMainClientId; }
	void SetPlayerObjectOfClient(int nClientId);

	void SetConnected(bool bConnected) { m_bConnected = bConnected; }
	bool IsConnected() const { return m_bConnected; }

	bool IsTcpClient() const { return m_bTcpClient; }
	void OnButtonClick(HWND hWnd);

	void SetMousePoint(POINT ptMouse) { m_ptOldCursorPos = ptMouse; }

	static int GetSwapChainNum() { return m_nSwapChainBuffers; }

	void SetGameState(int nGameState) { m_nGameState = nGameState; }

	//x == WIDTH , y == HEIGHT
	static POINT GetClientWindowSize();
private:

	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;
	//����Ʈ�� ���� �簢���̴�.

	HINSTANCE							m_hInstance;
	HWND								m_hWnd;

	//[0723] �������� ����� ���������ϵ��� ����
	static int									m_nWndClientWidth;
	static int									m_nWndClientHeight;

	ComPtr<IDXGIFactory4>				m_dxgiFactory;
	ComPtr<IDXGISwapChain3>				m_dxgiSwapChain;
	ComPtr<ID3D12Device>				m_d3d12Device;
	bool								m_bMsaa4xEnable = false;
	UINT								m_nMsaa4xQualityLevels = 0;

	static const UINT					m_nSwapChainBuffers = 2;
	UINT								m_nSwapChainBufferIndex;

	std::array<ComPtr<ID3D12Resource>, m_nSwapChainBuffers>			m_d3dSwapChainBackBuffers;
	ComPtr<ID3D12DescriptorHeap>									m_d3dRtvDescriptorHeap;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, m_nSwapChainBuffers>	m_pd3dSwapChainBackBufferRTVCPUHandles;

	ComPtr<ID3D12Resource>				m_d3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_d3dDsvDescriptorHeap;

	ComPtr<ID3D12CommandAllocator>		m_d3dCommandAllocator[m_nSwapChainBuffers];
	ComPtr<ID3D12CommandQueue>			m_d3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_d3dCommandList;

	ComPtr<ID3D12Fence>						m_d3dFence;
	std::array<UINT64, m_nSwapChainBuffers>	m_nFenceValues;
	HANDLE									m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug*						m_pd3dDebugController;
#endif

	//CGameTimer							m_GameTimer;
	
	shared_ptr<CScene>					m_pScene;

	std::array<shared_ptr<CPlayer>, MAX_CLIENT>	m_apPlayer;		// Ŭ���̾�ƮID�� �ε����� �����ϴ�.
	weak_ptr<CCamera>							m_pCamera;

	POINT								m_ptOldCursorPos;
	_TCHAR								m_pszFrameRate[200];
	
	static UCHAR						m_pKeysBuffer[256];

	//TCPClient
	unique_ptr<CTcpClient>				m_pTcpClient;
	static int							m_nMainClientId;	// TcpClient���� �ް� �ȴ�. -> �÷��̾� 1��Ī���� �׸��� ���ؼ� �׷��ְ� �ϱ�����
public:
	void PrepareDrawText();
	void RenderTextUI();

	static std::shared_ptr<CPlayer>					m_pMainPlayer;	// Ŭ���̾�ƮID�� �ش��ϴ� �ε����� �ش� Ŭ���̾�Ʈ�� Main�÷��̾�� �����ȴ�

	static shared_ptr<CPlayer>& GetMainPlayer() {
		return m_pMainPlayer;
	}
private:
	//DrawText
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<IDWriteFactory> m_dWriteFactory;
	ComPtr<ID3D11Resource> m_wrappedBackBuffers[m_nSwapChainBuffers];
	ComPtr<ID2D1Factory3> m_d2dFactory;
	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[m_nSwapChainBuffers];
	ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;

	ComPtr<ID2D1SolidColorBrush> m_textBrush;
	ComPtr<IDWriteTextFormat> m_textFormat;

	//unique_ptr<TextObject> m_pTextobject;
	bool m_bPrepareDrawText = false;
public:
	static ComPtr<IDWriteTextFormat> m_idwGameCountTextFormat;
	static ComPtr<IDWriteTextFormat> m_idwSpeakerTextFormat;

	//// Time 
	//D3D12_GPU_DESCRIPTOR_HANDLE m_d3dTimeCbvGPUDescriptorHandle;
	//ComPtr<ID3D12Resource>		m_pd3dcbTime;
	//FrameTimeInfo* m_pcbMappedTime;
	void UpdateFrameworkShaderVariable();
//[0507]
private:
	bool m_bConnected = false;
	HWND m_hConnectButton;

	bool m_bTcpClient = false;
	UINT m_nEventCreateTcpClient;

	HWND m_hIPAddressEdit;

	_TCHAR m_pszIPAddress[16];

	// �ϴ� �κ� ������ IN_GAME���� ����
	int m_nGameState = GAME_STATE::IN_GAME;
	float m_fEndingElapsedTime = 0.0f;
	XMFLOAT4 m_xmf4EndFog = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dFramework_info_CbvGPUDescriptorHandle;
	ComPtr<ID3D12Resource> m_d3dFramework_info_Resource;
	CB_FRAMEWORK_INFO* m_cbFramework_info;

	float m_fBGMVolume = 0.5f;
};


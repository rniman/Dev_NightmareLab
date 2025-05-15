#pragma once
#include "Timer.h"
#include "Shader.h"
#include "TCPClient.h"
#include "TextureBlendObject.h"

constexpr UINT WM_CHANGE_SLOT{ WM_USER + 5 };

// m_vShader ���̴��� AddDefaultObject �ÿ� ������ �� ���̴� �ε����� �ǹ�
#define STANDARD_SHADER 0
#define INSTANCE_STANDARD_SHADER 1
#define SKINNEDANIMATION_STANDARD_SHADER 2

// m_vForwardRenderShader
#define TRANSPARENT_SHADER 0 // ����ü�� ���� ���̴��� �׻� �ļ����� ��ġ
#define PARTICLE_SHADER 1 
#define TEXTUREBLEND_SHADER 2
#define TRAIL_SHADER 3
#define USER_INTERFACE_SHADER 4
#define OUT_LINE_SHADER 5

// m_vPartitionShader
#define PARTITION_SHADER 0


//#define NOTRENDERING_SHADER 3

// m_vMesh �޽��� ������ �� �ε����� �ǹ�
#define HEXAHEDRONMESH 0

#define MAX_LIGHTS						24 + MAX_SURVIVOR

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

#define WALK_SOUND_DISTANCE 16.0f

struct LIGHT
{
	XMFLOAT4X4							m_xmf4x4ViewProjection = Matrix4x4::Identity();
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	bool								m_bEnable = false;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS
{
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
	float bias;
};

struct FrameTimeInfo {
	float time = 0.0f;
	float localTime = 0.0f;
	float usePattern = -1.0f; // shaders���� �����ؽ�ó�� ����ϴ°�? 0���� ū���̸� ����ϴ� ��. ����ȭ �ʿ�. ���̴��� ������ �б⹮ ���ϼ�����.

	float fTrackingTime = 0.0f;

	// Occlusion Info
	float gfScale = 2.0f;
	float gfBias = 0.01f;
	float gfIntesity = 5.0f;
};

class CPlayer;
class CLoadedModelInfo;
class CTeleportObject;

class CScene
{
public:
	CScene() {};
	virtual ~CScene() {};

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) { return false; }
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) { return false; }

	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId) {}

	virtual void ReleaseUploadBuffers() {}

	virtual bool ProcessInput(UCHAR* pKeysBuffer) { return false; }
	virtual void AnimateObjects(float fElapsedTime, float fCurTime) {}
	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera) {}
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState) {}
	virtual void LoadingRender(ID3D12GraphicsCommandList* pd3dCommandList) {}

	// Interface
	void SetPlayer(shared_ptr<CPlayer> pPlayer, int nIndex) { m_apPlayer[nIndex] = pPlayer; }
	void SetMainPlayer(const shared_ptr<CPlayer>& pMainPlayer) { m_pMainPlayer = pMainPlayer; }

	void SetRTVDescriptorHeap(const ComPtr<ID3D12DescriptorHeap>& d3dRtvDescriptorHeap) { m_d3dRtvDescriptorHeap = d3dRtvDescriptorHeap; }
	void SetNumOfSwapChainBuffers(UINT nSwapChainBuffers) { m_nSwapChainBuffers = nSwapChainBuffers; }

	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature() { return m_pd3dGraphicsRootSignature; }

	std::array<shared_ptr<CPlayer>, MAX_CLIENT> m_apPlayer;
	std::shared_ptr<CPlayer> m_pMainPlayer;
protected:
	UINT m_nSwapChainBuffers;
	ComPtr<ID3D12DescriptorHeap> m_d3dRtvDescriptorHeap;

	ComPtr<ID3D12RootSignature>	m_pd3dGraphicsRootSignature;
	//��Ʈ �ñ׳�ó�� ��Ÿ���� �������̽� ������
	
	// �� ����� �̸�

protected:
	static ComPtr<ID3D12DescriptorHeap> m_pd3dCbvSrvUavDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dUavCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dUavGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dUavCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dUavGPUDescriptorNextHandle;
public:
	// Cbv Count
	static int m_nCntCbv;
	static int m_nCntSrv;
	static int m_nCntUav;

	static void CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, const shared_ptr<CTexture>& pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat);
	
	static void CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, const shared_ptr<CTexture>& pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);


	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUavDescriptorStartHandle() { return(m_d3dUavCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUavDescriptorStartHandle() { return(m_d3dUavGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUavDescriptorNextHandle() { return(m_d3dUavCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUavDescriptorNextHandle() { return(m_d3dUavGPUDescriptorNextHandle); }
public:
	virtual void SetParticleTest(float fCurTime) {}
	virtual void ParticleReadByteTask() {}
};

/// <CScene>
/////////////////////////////////////////////////////////////////////
/// <CScene - CLobbyScene>

class CLobbyScene : public CScene
{
public:
	CLobbyScene(HWND hWnd, weak_ptr<CCamera>& pCamera);
	virtual ~CLobbyScene() {};

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ProcessButtonDown(const POINT& ptCursorPos, std::shared_ptr<CLobbyUserInterfaceShader>& pLobbyUIShader);
	void ProcessClickBorder(const POINT& ptCursorPos, std::shared_ptr<CLobbyUserInterfaceShader>& pLobbyUIShader);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId) override;

	virtual void AnimateObjects(float fElapsedTime, float fCurTime) override;
	virtual bool ProcessInput(UCHAR* pKeysBuffer) override;
	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState) override;
	void LoadingRender(ID3D12GraphicsCommandList* pd3dCommandList) override;

	bool CheckCursor(POINT ptCursor, float fCenterX, float fCenterY, float fWidth, float fHeight);

	void UpdateShaderMainPlayer(int nMainClientId);

	int GetSelectedSlot()const { return m_nSelectedSlot; }
private:
	HWND m_hWnd;

	enum LOBBY_SHADER
	{
		LOBBY_SATANDARD_SHADER = 0,
		LOBBY_UI_SHADER
	};

	shared_ptr<CCamera> m_pCamera;

	vector<shared_ptr<CShader>> m_vpShader;

	POINT m_ptCursor;
	int m_nClientSlot = -1;
	int m_nSelectedSlot = -1;

private:
	unique_ptr<CFullScreenProcessingShader> m_vFullScreenProcessingShader;
};

/// <CScene - CLobbyScene>
/////////////////////////////////////////////////////////////////////
/// <CScene - CMainScene>

class CBlurComputeShader;
class CTextureToScreenShader;

class CMainScene : public CScene
{
public:
	CMainScene();
	virtual ~CMainScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId) override;
	void LoadScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	//������Ʈ �Ҹ� ����
	void ReleaseObjects();
	void ReleaseShaderVariables();
	void ReleaseUploadBuffers() override;

	//�� ������Ʈ ����
	virtual bool ProcessInput(UCHAR* pKeysBuffer) override;
	virtual void AnimateObjects(float fElapsedTime, float fCurTime) override;

	//[0626] ����Ʈ ���μ��� ���̴��� ������ ���鼭 gameframework�� PrevRenderTask �ڵ� ����
	void PrevRenderTask(ID3D12GraphicsCommandList* pd3dCommandList);

	//������ ����
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera,int nPipelineState) override;
	void ShadowPreRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState);

	//[0626] ����Ʈ ���μ��� ���̴��� ������ ���鼭 gameframework�� frame advance �ڵ� ����
	void ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState);
	void FinalRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, int nGameState);

	void ForwardRender(int nGameState, ID3D12GraphicsCommandList* pd3dCommandList, const std::shared_ptr<CCamera>& pCamera);

	void BlurDispatch(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle);
	
	void AddDefaultObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectType type, XMFLOAT3 position,int shader, int mesh);
	
	// �� ����
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dLightCbvGPUDescriptorHandle;
	LIGHT*								m_pLights = nullptr;
	int									m_nLights = 0;
	ComPtr<ID3D12Resource>				m_pd3dcbLights;
	LIGHTS*								m_pcbMappedLights = NULL;
	XMFLOAT4							m_xmf4GlobalAmbient;
	vector<XMFLOAT3>					m_xmf3lightPositions, m_xmf3lightLooks;
	vector<XMFLOAT3>& GetLightPositions() { return m_xmf3lightPositions; }
	vector<XMFLOAT3>& GetLightLooks() { return m_xmf3lightLooks; }

	vector<shared_ptr<CLightCamera>> m_pLightCamera;
	vector<shared_ptr<CLightCamera>>& GetLightCamera() { return m_pLightCamera; }

	void BuildLights(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	//void SetPlayer(shared_ptr<CPlayer> pPlayer, int nIndex);
	//void SetMainPlayer(const shared_ptr<CPlayer>& pMainplayer);

	//�� �� ������Ʈ(���̴�)
	static vector<unique_ptr<CShader>> m_vShader;
	vector<unique_ptr<CShader>> m_vForwardRenderShader;
	
	vector<shared_ptr<TextureBlendObject>> m_vTextureBlendObjects;
	shared_ptr<CMaterial> mt_Electirc;
	// ��Ƽ������ Com ��ü�� ������.�ؽ�ó�� ���ҽ��� �����Ǵµ� �� ��ü�� ���������� �����ϰ� ������� ������ �˾Ƽ� ������ �Ǹ鼭
	// ����̽����� ������ �߻� ��Ų��. 

	vector<unique_ptr<CShader>> m_vPreRenderShader;

	//[0626] gameframework���� scene���� �ű�
	CPostProcessingShader* m_pPostProcessingShader = NULL;
	int	m_nPostPipelineIndex = 0;

	//�޽� ����
	vector<shared_ptr<CMesh>>			m_vMesh;

	float m_fElapsedTime = 0.0f;

	float GetScale() { return m_pcbMappedTime->gfScale; }
	float GetIntesity() { return m_pcbMappedTime->gfIntesity; }
	float GetBias() { return m_pcbMappedTime->gfBias; }
	//void CreateComputeRootSignature(ID3D12Device* pd3dDevice);

	shared_ptr<CBlurComputeShader> m_pBlurComputeShader;
	shared_ptr<CTextureToScreenShader> m_pTextureToScreenShaderShader;
	unique_ptr<CFullScreenProcessingShader> m_vFullScreenProcessingShader;
private:
	// FrameInfo Time 
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dTimeCbvGPUDescriptorHandle;
	ComPtr<ID3D12Resource>		m_pd3dcbTime;
	FrameTimeInfo* m_pcbMappedTime;
public:
	void SetParticleTest(float fCurTime);
	void ParticleReadByteTask();
	void FullScreenProcessingRender(ID3D12GraphicsCommandList* pd3dCommandList);
};


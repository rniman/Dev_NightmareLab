//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "GameFramework.h"
#include "Player.h"
#include "Collision.h"
#include "SharedObject.h"
#include "Sound.h"

 extern UINT gnCbvSrvUavDescriptorIncrementSize;
 extern UINT gnRtvDescriptorIncrementSize;
 extern UINT gnDsvDescriptorIncrementSize;

 int CGameFramework::m_nWndClientWidth;
 int CGameFramework::m_nWndClientHeight;
 ComPtr<IDWriteTextFormat> CGameFramework::m_idwGameCountTextFormat;
 ComPtr<IDWriteTextFormat> CGameFramework::m_idwSpeakerTextFormat;

 UCHAR CGameFramework::m_pKeysBuffer[256] = {};
 int CGameFramework::m_nMainClientId = -1;

 float textX = 0.0f, textY = 0.0f;
 /////////////////////////////////////////////////////////
 //��𼭵� �����Ҽ� �ֵ����Ѵ�.
 std::shared_ptr<CPlayer> CGameFramework::m_pMainPlayer;
 /////////////////////////////////////////////////////////

 CGameFramework::CGameFramework()
{
	m_nSwapChainBufferIndex = 0;

	m_hFenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_d3dViewport = { 0,0,FRAME_BUFFER_WIDTH,FRAME_BUFFER_HEIGHT,0.0f,1.0f };
	m_d3dScissorRect = { 0,0,FRAME_BUFFER_WIDTH,FRAME_BUFFER_HEIGHT };

	m_pScene = NULL;

	m_pTcpClient = make_unique<CTcpClient>();

	_tcscpy_s(m_pszFrameRate, _T("NightMare Lab ("));
}

CGameFramework::~CGameFramework()
{
#ifndef SINGLE_PLAY
	m_pClientNetwork->Exit();
#endif // SINGLE_PLAY
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	ChangeSwapChainState();
#endif

	CoInitialize(NULL);
	
	SoundManager& soundManager = SoundManager::GetInstance();
	soundManager.Initialize();

	//m_pTcpClient = make_shared<CTcpClient>(hMainWnd);

	//g_collisionManager.CreateCollision(SPACE_FLOOR, SPACE_WIDTH, SPACE_DEPTH);
	BuildObjects();
	//[0514] PrepaerDrawText-> â��� ��ȯ�� ��������
	//PrepareDrawText();// Scene�� �ʱ�ȭ �ǰ� ���� �����ؾ��� SRV�� Scene�� ������ ����.

	return(true);
}
//#define _WITH_CREATE_SWAPCHAIN_FOR_HWND

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	//m_nWndClientWidth = rcClient.right - rcClient.left;
	//m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_dxgiFactory->CreateSwapChainForHwnd(m_d3dCommandQueue.Get(), m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)m_dxgiSwapChain.GetAddressOf());
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_dxgiFactory->CreateSwapChain(m_d3dCommandQueue.Get(), &dxgiSwapChainDesc, (IDXGISwapChain**)m_dxgiSwapChain.GetAddressOf());
#endif
	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)m_dxgiFactory.GetAddressOf());

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)m_d3d12Device.GetAddressOf()))) break;
	}

	if (!pd3dAdapter)
	{
		m_dxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)m_d3d12Device.GetAddressOf());
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)m_d3dFence.GetAddressOf());
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvUavDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_d3d12Device->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)m_d3dCommandQueue.GetAddressOf());

	//hResult = m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)m_d3dCommandAllocator.GetAddressOf());

	for (int i = 0;i < m_nSwapChainBuffers;++i) {
		hResult = m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)m_d3dCommandAllocator[i].GetAddressOf());
		//ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_d3dCommandAllocator[i])));
	}
	hResult = m_d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void**)m_d3dCommandList.GetAddressOf());
	hResult = m_d3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + ADD_RENDERTARGET_COUNT + 2 ;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_d3d12Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_d3dRtvDescriptorHeap.GetAddressOf());
	::gnRtvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_d3d12Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_d3dDsvDescriptorHeap.GetAddressOf());
	::gnDsvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	HRESULT hResult;

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Ŭ���� ���� �� ����
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // ������ (���� ������)
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		hResult = m_dxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_d3dSwapChainBackBuffers[i].GetAddressOf());
		m_d3d12Device->CreateRenderTargetView(m_d3dSwapChainBackBuffers[i].Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dSwapChainBackBufferRTVCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_d3d12Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, __uuidof(ID3D12Resource), (void**)m_d3dDepthStencilBuffer.GetAddressOf());

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3d12Device->CreateDepthStencilView(m_d3dDepthStencilBuffer.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	HRESULT hResult;
	BOOL bFullScreenState = FALSE;
	hResult = m_dxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	hResult = m_dxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	hResult = m_dxgiSwapChain->GetDesc(&dxgiSwapChainDesc);

	DXGI_MODE_DESC dxgiTargetParameters;
	::ZeroMemory(&dxgiTargetParameters, sizeof(dxgiTargetParameters));
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	hResult = m_dxgiSwapChain->ResizeTarget(&dxgiSwapChainDesc.BufferDesc);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_d3dSwapChainBackBuffers[i])
		{
			m_d3dSwapChainBackBuffers[i].Reset();
		}
	}

	//[0514] ����� ���� ���־���
	if (m_bPrepareDrawText)
	{
		m_d3d11DeviceContext.Reset();
		m_d3d11On12Device.Reset();
		//m_dWriteFactory.Reset();
		m_wrappedBackBuffers[0].Reset();
		m_wrappedBackBuffers[1].Reset();
		m_d2dFactory.Reset();
		m_d2dDevice.Reset();
		m_d2dRenderTargets[0].Reset();
		m_d2dRenderTargets[1].Reset();
		m_d2dDeviceContext.Reset();

		m_textBrush.Reset();
		//m_textFormat.Reset();
		//m_idwGameCountTextFormat.Reset();
	}

	hResult = m_dxgiSwapChain->ResizeBuffers(2, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();

	//[0514] ����� ���� ���־���
	if (m_bPrepareDrawText)
	{
		PrepareDrawText();
	}
}

void CGameFramework::PrepareDrawText()
{
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {}; //drawText
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	//DrawText
	ComPtr<ID3D11Device> d3d11Device;
	D3D11On12CreateDevice(
		m_d3d12Device.Get(),
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_d3dCommandQueue.GetAddressOf()),
		1,
		0,
		d3d11Device.GetAddressOf(),
		m_d3d11DeviceContext.GetAddressOf(),
		nullptr
	);

	ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));
	{
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
		ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext));
		if (!m_dWriteFactory) {
			ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
		}
	}

	float dpiX;
	float dpiY;

#pragma warning(push)
#pragma warning(disable : 4996) // GetDesktopDpi is deprecated.
	m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);

	//[CJI 0412] �ؽ�Ʈ�� ����Ÿ�ٿ� �׸��� �̸� �ؽ�ó�� �ٲ����� �ٸ� �繰�� �����Ϸ��� ������ ����(�ð��ʹ� ��� �н�).. ���߿� �ѹ� �غ���. 
	//m_pTextobject = make_unique<TextObject>(m_d3d12Device.Get(), m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers);

	for (UINT n = 0; n < m_nSwapChainBuffers; n++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
			m_d3dSwapChainBackBuffers[n].Get(),
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_wrappedBackBuffers[n])
		));

		ComPtr<IDXGISurface> surface;
		ThrowIfFailed(m_wrappedBackBuffers[n].As(&surface));
		ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
			surface.Get(),
			&bitmapProperties,
			&m_d2dRenderTargets[n]
		));

	}

	{
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &m_textBrush));

		if (m_dWriteFactory && !m_textFormat) {
			ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
				L"Verdana",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				50,
				L"ko-KR",
				&m_textFormat
			));
			ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
			ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

			ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
				L"�ü�",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				400.f,
				L"ko-KR",
				&m_idwGameCountTextFormat
			));
			ThrowIfFailed(m_idwGameCountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
			ThrowIfFailed(m_idwGameCountTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

			ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
				L"�ü�",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				35,
				L"ko-KR",
				&m_idwSpeakerTextFormat
			));
			ThrowIfFailed(m_idwSpeakerTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
			ThrowIfFailed(m_idwSpeakerTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

		}
		
	}

	m_bPrepareDrawText = true;
}

//float uiX{}, uiY{};

void CGameFramework::RenderTextUI()
{
	if (m_nGameState != GAME_STATE::IN_GAME)
	{
		return;
	}

	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();

	// ���� �� ���ۿ� ���� ���ε� ���� Ÿ�� �ڿ��� ȹ���մϴ�.
	m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);
	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_nSwapChainBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();

	m_pMainPlayer->RenderTextUI(m_d2dDeviceContext, m_textFormat, m_textBrush);

	ThrowIfFailed(m_d2dDeviceContext->EndDraw());
	// ���ε� ���� Ÿ�� �ڿ��� �����մϴ�. �����ϸ� ���ε� �ڿ��� ������ �� ������ OutState�� �� ���� �ڿ��� ��ȯ�˴ϴ�
	m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_nSwapChainBufferIndex].GetAddressOf(), 1);
	//��� ����� ���� ��� ť�� �����ϱ� ���� �÷����մϴ�.
	m_d3d11DeviceContext->Flush();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (!m_bConnected)	// ������ ���� X
	{

		return;
	}

	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);

	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		return;
	}

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		SetCursor(NULL);
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}
bool TESTBOOL = true;
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (!m_bConnected)
	{
		switch (nMessageID)
		{
		case WM_KEYUP:
			switch (wParam)
			{
			case VK_ESCAPE:
				::PostQuitMessage(0);
				break;
			default:
				break;
			}
			break;
		default:
			break;

		}
		return;
	}

	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_UP: 
		{
			//sharedobject.AddParticle(CParticleMesh::FOOTPRINT, XMFLOAT3());
			//m_pScene->SetParticleTest(gGameTimer.GetTotalTime());
			//m_pMainPlayer->SetHitRender(true);
			//textX += 10.f;
			TESTBOOL = false;
			break;
		}
		case VK_DOWN: 
			textY += 10.f;
			break;
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_CONTROL:	// ĸ�� ����
			if (GetCapture())
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				::ReleaseCapture();
			}
			else
			{
				SetCursor(NULL);
				SetCapture(hWnd);
				SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
			}
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
			if (!m_pMainPlayer)
			{
				break;
			}
			m_pMainPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), gGameTimer.GetTimeElapsed());
			m_pCamera = m_pMainPlayer->GetCamera();
			break;
		case VK_F3:
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingCommandMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == BUTTON_CREATE_TCP_ID)
		{
			OnButtonClick(hWnd);
		}
		break;
	}
}

void CGameFramework::OnProcessingSocketMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	// ���� �߻� ���� Ȯ��
	if (WSAGETSELECTERROR(lParam))
	{
		err_display(WSAGETSELECTERROR(lParam));
		return;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_WRITE:	// ������ �����͸� ������ �غ� �Ǿ���.
	case FD_READ:	// ������ �����͸� ���� �غ� �Ǿ���.
	case FD_CLOSE:
		m_pTcpClient->OnProcessingSocketMessage(hWnd, nMessageID, wParam, lParam);
		if (WSAGETSELECTEVENT(lParam) == FD_CLOSE && m_bTcpClient)
		{
			m_bTcpClient = false;
			err_display("Fail Connect", "Client count exceeded or game already started");
		}
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_COMMAND:
		OnProcessingCommandMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_CHANGE_SLOT:
	{
		shared_ptr<CLobbyScene> pScene = dynamic_pointer_cast<CLobbyScene>(m_pScene);
		if (LOWORD(wParam) == 0) // ��ȯ ��ü
		{
			m_pTcpClient->SetSocketState(SOCKET_STATE::SEND_CHANGE_SLOT);
			INT8 nSelectedSlot = pScene->GetSelectedSlot();
			m_pTcpClient->SetSelectedSlot(nSelectedSlot);
			PostMessage(m_hWnd, WM_SOCKET, NULL, MAKELPARAM(FD_WRITE, 0));

			INT8 nChangeID = m_apPlayer[nSelectedSlot]->GetClientId();
			m_apPlayer[m_nMainClientId]->SetClientId(nChangeID);
			m_nMainClientId = nSelectedSlot;
			m_apPlayer[m_nMainClientId]->SetClientId(m_nMainClientId);
			m_pMainPlayer = m_apPlayer[m_nMainClientId];
			pScene->SetMainPlayer(m_pMainPlayer);
			pScene->UpdateShaderMainPlayer(m_nMainClientId);
		}
		else if (LOWORD(wParam) == 1) // ��ȯ ����
		{
			INT8 nChangeID = m_pTcpClient->GetMainClientId();
			m_nMainClientId = nChangeID;
			m_apPlayer[m_nMainClientId]->SetClientId(m_nMainClientId);
			m_pMainPlayer = m_apPlayer[m_nMainClientId];
			pScene->SetMainPlayer(m_pMainPlayer);
			pScene->UpdateShaderMainPlayer(m_nMainClientId);
		}
	}
		break;
	case WM_CREATE_TCP:
		m_bTcpClient = true;
		break;
	case WM_START_GAME:
	{
		m_nGameState = GAME_STATE::IN_LOADING;

		//// �ε�ȭ��
		LoadingRender();
		////
		BuildObjects();

		//::SetCursor(NULL);
		//::SetCapture(hWnd);
		//// ���콺�� ȭ�� �߾����� �̵���Ŵ (������ ���ηθ� �̵��ϵ���)
		RECT rect;
		GetClientRect(hWnd, &rect);
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(hWnd, &center);
		//SetCursorPos(center.x, center.y);
		SetMousePoint(center);
		
		m_apPlayer[m_nMainClientId]->SetGameStart();

		//�ε� �Ϸ� �޽��� Send
		m_pTcpClient->LoadCompleteSend();
	}
	break;
	case WM_END_GAME:
		OnProcessingEndGameMessage(wParam);
		break;
	case WM_SOCKET:
		OnProcessingSocketMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			gGameTimer.Stop();
		else
			gGameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
}

void CGameFramework::OnProcessingEndGameMessage(WPARAM& wParam)
{
	SoundManager& soundManager = soundManager.GetInstance();
	
	if (LOWORD(wParam) == 0)	// BLUESUIT WIN
	{
		m_nGameState = GAME_STATE::BLUE_SUIT_WIN;

		if (m_nMainClientId == ZOMBIEPLAYER)
		{
			soundManager.PlaySoundWithName(sound::GAME_OVER, -1);
			soundManager.SetVolume(sound::GAME_OVER, 0.25f);
		}
		else
		{
			soundManager.PlaySoundWithName(sound::GAME_WIN, -1);
			soundManager.SetVolume(sound::GAME_WIN, 0.25f);
		}
	}
	else if (LOWORD(wParam) == 1)	// ZOMBIE WIN
	{
		m_nGameState = GAME_STATE::ZOMBIE_WIN;
		for (int i = ZOMBIEPLAYER + 1; i < ZOMBIEPLAYER + 1 + 4; ++i)
		{
			m_apPlayer[i]->SetAlive(false);
		}

		if (m_nMainClientId == ZOMBIEPLAYER)
		{
			soundManager.PlaySoundWithName(sound::GAME_WIN, -1);
			soundManager.SetVolume(sound::GAME_WIN, 0.25f);
		}
		else
		{
			soundManager.PlaySoundWithName(sound::GAME_OVER, -1);
			soundManager.SetVolume(sound::GAME_OVER, 0.25f);
		}
	}

	shared_ptr<CMainScene> pMainScene = dynamic_pointer_cast<CMainScene>(m_pScene);
	if (m_nMainClientId != ZOMBIEPLAYER)
	{
		dynamic_cast<CBlueSuitUserInterfaceShader*>(pMainScene->m_vForwardRenderShader[USER_INTERFACE_SHADER].get())->SetGameState(m_nGameState);
	}
	else if (m_nMainClientId == ZOMBIEPLAYER)
	{
		dynamic_cast<CZombieUserInterfaceShader*>(pMainScene->m_vForwardRenderShader[USER_INTERFACE_SHADER].get())->SetGameState(m_nGameState);
	}
}

UCHAR* CGameFramework::GetKeysBuffer()
{
	return m_pKeysBuffer;
}

void CGameFramework::SetPlayerObjectOfClient(int nClientId)
{
	m_nMainClientId = nClientId;
	m_pMainPlayer = m_apPlayer[nClientId];
	m_pMainPlayer->GetCamera()->SetPlayer(m_pMainPlayer);
	m_pCamera = m_pMainPlayer->GetCamera();
	g_collisionManager.m_pPlayer = m_pMainPlayer;

	m_pScene->SetMainPlayer(m_pMainPlayer);
}

void CGameFramework::OnButtonClick(HWND hWnd)
{
	GetWindowText(m_hIPAddressEdit, m_pszIPAddress, 20);

	if (m_pTcpClient->CreateSocket(hWnd, m_pszIPAddress))
	{
		SendMessage(hWnd, WM_CREATE_TCP, NULL, NULL);
	}
}

POINT CGameFramework::GetClientWindowSize()
{
	return POINT(m_nWndClientWidth, m_nWndClientHeight);
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_dxgiSwapChain)
	{
		m_dxgiSwapChain->SetFullscreenState(FALSE, NULL);
	}

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif

}

void CGameFramework::OnDestroyEntryWindow()
{
	DestroyWindow(m_hConnectButton);
	DestroyWindow(m_hIPAddressEdit);
}

void CGameFramework::CreateEntryWindow(HWND hWnd)
{
	int nConnectButtonWidth = 300;
	int nConnectButtonHeight = 60;

	m_hConnectButton = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"BUTTON", // Ŭ���� �̸�
		L"Connect to Server", // �ؽ�Ʈ
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // ��Ÿ��
		FRAME_BUFFER_WIDTH / 2 - nConnectButtonWidth / 2, FRAME_BUFFER_HEIGHT / 2 + nConnectButtonHeight, nConnectButtonWidth, nConnectButtonHeight, // ��ġ�� ũ��
		hWnd, // �θ� ������
		(HMENU)BUTTON_CREATE_TCP_ID, // ��ư ID
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);

	int nIPEditWidth = 400;
	int nIPEditHeight = 50;
	m_hIPAddressEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER,
		FRAME_BUFFER_WIDTH / 2 - nIPEditWidth / 2, FRAME_BUFFER_HEIGHT / 2, nIPEditWidth, nIPEditHeight,
		hWnd,
		(HMENU)EDIT_INPUT_ADDRESS_ID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);
	SendMessage(m_hIPAddressEdit, EM_LIMITTEXT, (WPARAM)16, 0);	// 16���� ����


	HFONT hFont = CreateFont(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	SendMessage(m_hIPAddressEdit, WM_SETFONT, (WPARAM)hFont, TRUE);


	int nIPCaptionWidth = 400;
	int nIPCaptionHeight = 40;
	HWND hLabel = CreateWindowEx(
		0,
		L"STATIC",
		L"IP ADDRESS",
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		FRAME_BUFFER_WIDTH / 2 - nIPCaptionWidth / 2, FRAME_BUFFER_HEIGHT / 2 - nIPCaptionHeight, nIPCaptionWidth, nIPCaptionHeight,
		hWnd,
		(HMENU)EDIT_INPUT_ADDRESS_ID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL
	);
	SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void CGameFramework::BuildObjects()
{
	gGameTimer.Reset();

	SoundManager& soundManager = soundManager.GetInstance();
	
	m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);
	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		m_fBGMVolume = 0.25f;
		soundManager.PlaySoundWithName(sound::LOBBY_SCENE, -1);
		soundManager.SetVolume(sound::LOBBY_SCENE, m_fBGMVolume);

		m_pScene = make_shared<CLobbyScene>(m_hWnd, m_pCamera);
		m_pScene->SetNumOfSwapChainBuffers(m_nSwapChainBuffers);
		m_pScene->SetRTVDescriptorHeap(m_d3dRtvDescriptorHeap);

		int nMainClientId = m_pTcpClient->GetMainClientId();
		m_pScene->BuildObjects(m_d3d12Device.Get(), m_d3dCommandList.Get(), nMainClientId);
		m_pCamera.lock()->CreateShaderVariables(m_d3d12Device.Get(), m_d3dCommandList.Get());

		for (int i = 0; i < MAX_CLIENT; ++i)
		{
			m_apPlayer[i] = m_pScene->m_apPlayer[i];
			m_pTcpClient->SetPlayer(m_pScene->m_apPlayer[i], i);
			int nClientId = m_pTcpClient->GetClientID(i);
			m_apPlayer[i]->SetClientId(nClientId);
		}
		m_nMainClientId = nMainClientId;
		m_pMainPlayer = m_apPlayer[nMainClientId];
		m_pScene->SetMainPlayer(m_pMainPlayer);

		m_d3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
		m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();
	}
	else if (m_nGameState == GAME_STATE::IN_LOADING)
	{
		m_fBGMVolume = 0.07f;
		soundManager.StopSound(sound::LOBBY_SCENE);
		soundManager.PlaySoundWithName(sound::MAIN_SCENE, -1);
		soundManager.SetVolume(sound::MAIN_SCENE, m_fBGMVolume);

		g_collisionManager.CreateCollision(SPACE_FLOOR, SPACE_WIDTH, SPACE_DEPTH);

		m_pScene = make_shared<CMainScene>();
		m_pScene->SetNumOfSwapChainBuffers(m_nSwapChainBuffers);
		m_pScene->SetRTVDescriptorHeap(m_d3dRtvDescriptorHeap);

		shared_ptr<CMainScene> pMainScene = dynamic_pointer_cast<CMainScene>(m_pScene);
		if (m_pScene.get())
		{
			int nMainClientId = m_pTcpClient->GetMainClientId();
			m_pScene->BuildObjects(m_d3d12Device.Get(), m_d3dCommandList.Get(), nMainClientId);

			for (int i = 0; i < MAX_CLIENT; ++i)
			{
				m_apPlayer[i] = m_pScene->m_apPlayer[i];
				m_pTcpClient->SetPlayer(m_pScene->m_apPlayer[i], i);
				int nClientId = m_pTcpClient->GetClientID(i);
				m_apPlayer[i]->SetClientId(nClientId);
			}

			m_nMainClientId = nMainClientId;
			m_pMainPlayer = m_apPlayer[nMainClientId];
			m_pScene->SetMainPlayer(m_pMainPlayer);
			
			m_pMainPlayer->SetPlayerVolume(1.0f);
		}

		UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); //256�� ���
		m_d3dFramework_info_Resource = ::CreateBufferResource(m_d3d12Device.Get(), m_d3dCommandList.Get(), NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
		m_d3dFramework_info_Resource->Map(0, NULL, (void**)&m_cbFramework_info);
		m_d3dFramework_info_CbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(m_d3d12Device.Get(), 1, m_d3dFramework_info_Resource.Get(), ncbElementBytes);

		m_d3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
		m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();

		if (m_pScene)
		{
			m_pScene->ReleaseUploadBuffers();
		}

		PreRenderTasks(pMainScene); // ���� ������ �۾�

		int light_id = 0;
		auto& LightCamera = pMainScene->GetLightCamera();
		for (auto& pPlayer : m_apPlayer)
		{
			pPlayer->ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
			pPlayer->Update(gGameTimer.GetTimeElapsed());

			auto survivor = dynamic_pointer_cast<CBlueSuitPlayer>(pPlayer);
			if (survivor) {
				LightCamera[light_id]->SetPlayer(pPlayer);
				light_id++;
			}
		}
		m_pCamera = m_pMainPlayer->GetCamera();

		PrepareDrawText();// Scene�� �ʱ�ȭ �ǰ� ���� �����ؾ��� SRV�� Scene�� ������ ����.

		int x = g_collisionManager.GetNumOfCollisionObject();
		if (x != 0) {
			x = x;
		}
	}
}

void CGameFramework::ReleaseObjects()
{
	//if (m_pScene) m_pScene->ReleaseObjects();
	//if (m_pScene) delete m_pScene;
}

void CGameFramework::ProcessInput()
{
	bool bProcessedByScene = false;
	GetKeyboardState(m_pKeysBuffer);
	if (!m_pMainPlayer)
	{
		return;
	}

	if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		m_pScene->ProcessInput(m_pKeysBuffer);
		return;
	}

	//if ( && m_pScene) bProcessedByScene = m_pScene->ProcessInput(m_pKeysBuffer);
	PostMessage(m_hWnd, WM_SOCKET, (WPARAM)m_pTcpClient->m_sock, MAKELPARAM(FD_WRITE, 0));

	if (!bProcessedByScene)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{

				if (m_pMainPlayer->m_bAlive || !m_pMainPlayer->m_pSkinnedAnimationController->IsAnimation())
				{
					m_pMainPlayer->Rotate(cyDelta, cxDelta, 0.0f);
				}
			}
		}
	}

	for (auto& pPlayer : m_apPlayer)
	{
		if (pPlayer->GetClientId() == -1)
		{
			continue;
		}
		pPlayer->Update(gGameTimer.GetTimeElapsed());
	}
}

void CGameFramework::AnimateObjects()
{
	float fElapsedTime = gGameTimer.GetTimeElapsed();

	if (m_pScene) m_pScene->AnimateObjects(fElapsedTime, gGameTimer.GetTotalTime());

	//vector<shared_ptr<CLightCamera>>& lightCamera = m_pScene->GetLightCamera();

	//XMFLOAT3 clientCameraPos = m_pCamera.lock().get()->GetPosition();
	//sort(lightCamera.begin() + 4, lightCamera.end(), [clientCameraPos](const shared_ptr<CLightCamera>& A, const shared_ptr<CLightCamera>& B) {
	//	//const float epsilon = 1e-5f; // ��� ����
	//	XMFLOAT3 clToA = Vector3::Subtract(clientCameraPos, A->GetPosition());
	//	XMFLOAT3 clToB = Vector3::Subtract(clientCameraPos, B->GetPosition());
	//	return Vector3::Length(clToA) < Vector3::Length(clToB);
	//	});

	//for (auto& cm : lightCamera) {
	//	cm->Update(cm->GetLookAtPosition(), fElapsedTime);
	//	if (auto player = cm->GetPlayer().lock()) {
	//		if (player->GetClientId() == -1) {
	//			cm->m_pLight->m_bEnable = false;
	//		}
	//	}
	//}
}

void CGameFramework::AnimateEnding()
{
	static bool bUpdateElevatorDoor = false;
	shared_ptr<CGameObject> pDoor = g_collisionManager.GetCollisionObjectWithNumber(m_pTcpClient->GetEscapeDoor()).lock();

	if (!bUpdateElevatorDoor)
	{
		g_collisionManager.GetCollisionObjectWithNumber(m_pTcpClient->GetEscapeDoor()).lock()->UpdatePicking();
		bUpdateElevatorDoor = true;
	}
	pDoor->Animate(gGameTimer.GetTimeElapsed());
	AnimateObjects();

}


void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_d3dCommandQueue->Signal(m_d3dFence.Get(), nFenceValue);

	if (m_d3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_d3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_d3dCommandQueue->Signal(m_d3dFence.Get(), nFenceValue);

	if (m_d3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_d3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::PreRenderTasks(shared_ptr<CMainScene>& pMainScene)
{
	INT8 nClientId = m_pTcpClient->GetMainClientId();
	if (nClientId != -1)
	{
		SetPlayerObjectOfClient(nClientId);
		//m_bPrevRender = true;
	}
	else	// ������
	{
		assert("FAIL CLIENT ID");
	}

	if (pMainScene->m_nLights >= MAX_LIGHTS)
	{
		pMainScene->m_nLights = MAX_LIGHTS;
	}
	m_pMainPlayer->Update(/*gGameTimer.GetTimeElapsed()*/0.01f);

	AnimateObjects();
	// �̰����� ������ �ϱ����� �غ��۾��� �����ϵ����Ѵ�. ex) ������� ����ŷ
	// buildobject�Լ� ȣ�� ���� ó���Ǿ���� �۾��̴�. -> ��� ��ü���� �������Ǿ�� �׸��ڸ��� ������.

	//HRESULT hResult = m_d3dCommandAllocator->Reset();
	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	/*FLOAT ClearValue[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_d3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);*/

	pMainScene->PrevRenderTask(m_d3dCommandList.Get());

	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_d3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	//m_dxgiSwapChain->Present(0, 0); // ���� �������� ���������� ����� ���� �����Ƿ� ȭ����ȯ x

	MoveToNextFrame();
}

void CGameFramework::LoadingRender()
{
	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);
	
	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	FLOAT ClearValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_d3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);
	m_d3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	m_pScene->LoadingRender(m_d3dCommandList.Get());

	SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_d3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	m_dxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	/*gGameTimer.GetFrameRate(m_pszFrameRate + 15, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = xmf3Position = m_pMainPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 200 - nLength, _T("ID:%d %d, NumOfClient: %d, (%4f, %4f, %4f), %d"), m_pTcpClient->GetMainClientId(), m_nMainClientId, m_pTcpClient->GetNumOfClient(), xmf3Position.x, xmf3Position.y, xmf3Position.z, g_collisionManager.GetNumOfCollisionObject());
	::SetWindowText(m_hWnd, m_pszFrameRate);*/
}

//#define _WITH_PLAYER_TOP
void CGameFramework::FrameAdvance()
{
	gGameTimer.Tick(60.0f);

	SoundManager& soundManager = SoundManager::GetInstance();
	soundManager.UpdateSystem();

	if (m_nGameState == GAME_STATE::IN_GAME)
	{
		ProcessInput();
		AnimateObjects();
		//soundManager.SetVolume(sound::LOBBY_SCENE, m_fBGMVolume);
	}
	else if (m_nGameState == GAME_STATE::IN_LOBBY)
	{
		ProcessInput();
		AnimateObjects();

		//soundManager.SetVolume(sound::MAIN_SCENE, m_fBGMVolume);
	}
	else if (m_nGameState == GAME_STATE::IN_LOADING) {
		//LoadingRender();
		if (m_pTcpClient->GetRecvLoadComplete()) {
			m_nGameState = GAME_STATE::IN_GAME;
		}
		return;
	}
	else
	{
		AnimateEnding();
		m_fEndingElapsedTime += gGameTimer.GetTimeElapsed();

		m_pMainPlayer->UpdateEnding(m_fEndingElapsedTime, m_nGameState);
	}

	HRESULT hResult = m_d3dCommandAllocator[m_nSwapChainBufferIndex]->Reset();
	hResult = m_d3dCommandList->Reset(m_d3dCommandAllocator[m_nSwapChainBufferIndex].Get(), NULL);

	// ������
	switch (m_nGameState)
	{
	case GAME_STATE::IN_LOBBY:
	{
		SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_d3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

		FLOAT ClearValue[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
		m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_d3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);
		m_d3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

		m_pScene->Render(m_d3dCommandList.Get(), m_pCamera.lock(), 0);

		SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	break;
	case GAME_STATE::IN_GAME:
	case GAME_STATE::BLUE_SUIT_WIN:
	case GAME_STATE::ZOMBIE_WIN:
	{
		shared_ptr<CMainScene> pMainScene = dynamic_pointer_cast<CMainScene>(m_pScene);

		pMainScene->ShadowRender(m_d3dCommandList.Get(), m_pCamera.lock(), 0);

		//�׸��ڸ� ������ ���������� ó��.
		for (auto& pl : m_apPlayer)
		{
			if (pl->GetClientId() == -1)
				continue;
			pl->SetShadowRender(false);
		}

		SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = pMainScene->m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);
		m_d3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		pMainScene->m_pPostProcessingShader->OnPrepareRenderTarget(m_d3dCommandList.Get(), 0, &m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBufferIndex], &d3dDsvCPUDescriptorHandle);

		pMainScene->PrepareRender(m_d3dCommandList.Get(), m_pCamera.lock());
		UpdateFrameworkShaderVariable();
		pMainScene->FinalRender(m_d3dCommandList.Get(), m_pCamera.lock(), d3dRtvCPUDescriptorHandle, m_nGameState);

		pMainScene->BlurDispatch(m_d3dCommandList.Get(), m_pCamera.lock(), d3dRtvCPUDescriptorHandle);
		pMainScene->ForwardRender(m_nGameState, m_d3dCommandList.Get(), m_pCamera.lock());
		pMainScene->FullScreenProcessingRender(m_d3dCommandList.Get());
	}
	break;
	default:
		break;
	}
	//SynchronizeResourceTransition(m_d3dCommandList.Get(), m_d3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//[CJI 0411] -> RenderUI���� ����Ÿ�� ����� ������ m_wrappedBackBuffers�� �ڿ��� �����Ҷ� �ڵ������� ���¸� D3D12_RESOURCE_STATE_PRESENT���� �ǵ����� ������ ���ʿ�

	hResult = m_d3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_d3dCommandList.Get() };
	m_d3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	RenderTextUI();

	WaitForGpuComplete();

	if (m_pScene) {
		m_pScene->ParticleReadByteTask();
	}

	m_dxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	gGameTimer.GetFrameRate(m_pszFrameRate + 15, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = xmf3Position = m_pMainPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 200 - nLength, _T("ID:%d %d, NumOfClient: %d, (%4f, %4f, %4f), %d"), m_pTcpClient->GetMainClientId(), m_nMainClientId, m_pTcpClient->GetNumOfClient(), xmf3Position.x, xmf3Position.y, xmf3Position.z, g_collisionManager.GetNumOfCollisionObject());
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::UpdateFrameworkShaderVariable()
{
	m_cbFramework_info->m_fCurrentTime = gGameTimer.GetTotalTime();
	m_cbFramework_info->m_fElapsedTime = gGameTimer.GetTimeElapsed();

	m_d3dCommandList->SetGraphicsRootDescriptorTable(14, m_d3dFramework_info_CbvGPUDescriptorHandle);
}
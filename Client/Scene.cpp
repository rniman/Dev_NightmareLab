#include "stdafx.h"
#include "Scene.h"
#include "Shader.h"
#include "ParticleShader.h"
#include "TextureBlendAnimationShader.h"
#include "Player.h"
#include "PlayerController.h"
#include "EnvironmentObject.h"
#include "Collision.h"
#include "TextureBlendObject.h"
#include "SharedObject.h"
#include "CTrailShader.h"
#include "BlurComputeShader.h"

ComPtr<ID3D12DescriptorHeap> CScene::m_pd3dCbvSrvUavDescriptorHeap;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavGPUDescriptorNextHandle;

vector<unique_ptr<CShader>> CMainScene::m_vShader;
//CShader* CScene::m_pRefShader;

extern bool g_InstanceMeshNotAddCollision;

int ReadLightObjectInfo(vector<XMFLOAT3>& positions, vector<XMFLOAT3>& looks);
void PartisionShaderCollision(unique_ptr<PartitionInsStandardShader>& PtShader, shared_ptr<CGameObject>& pObject);

void CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[18];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //b0 Camera: 
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1; //b1 GameObject: 
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2; //b2 Light?: 
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 0; //t0: AlbedoTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 1; //t1: SpecularTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 2; //t2: NormalTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 3; //t3: MetallicTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 4; //t4: EmissionTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 3; //b3: gpmtxBoneOffsets
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 4; //b4: gpmtxBoneTransforms
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = ADD_RENDERTARGET_COUNT;
	pd3dDescriptorRanges[10].BaseShaderRegister = 5; //t5,t6,t7,t8: Deferred Render Texture,  t9: emissive, t10: compute 
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 28;
	pd3dDescriptorRanges[11].BaseShaderRegister = 20; //t20~ Shadow Map
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 5; //b5
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 11; //t9 patterntexture -> t11
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[14].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[14].NumDescriptors = 1;
	pd3dDescriptorRanges[14].BaseShaderRegister = 6; //b5
	pd3dDescriptorRanges[14].RegisterSpace = 0;
	pd3dDescriptorRanges[14].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[15].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[15].NumDescriptors = 1;
	pd3dDescriptorRanges[15].BaseShaderRegister = 12; //t10 gRandomBuffer -> t12
	pd3dDescriptorRanges[15].RegisterSpace = 0;
	pd3dDescriptorRanges[15].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[16].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[16].NumDescriptors = 1;
	pd3dDescriptorRanges[16].BaseShaderRegister = 0; //u0: RWTexture2D
	pd3dDescriptorRanges[16].RegisterSpace = 0;
	pd3dDescriptorRanges[16].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[17].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[17].NumDescriptors = 1;
	pd3dDescriptorRanges[17].BaseShaderRegister = 14; //t13: ������ Rtv
	pd3dDescriptorRanges[17].RegisterSpace = 0;
	pd3dDescriptorRanges[17].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[18];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1; //Camera
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1; //GameObject
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Lights
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1; //AlbedoTexture
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1; //SpecularTexture
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1; //NormalTexture
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1; //MetallicTexture
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1; //EmissionTexture
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1; //gpmtxBoneOffsets
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1; //gpmtxBoneTransforms
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[10]; //Deferred Render Texture + HDR
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[11]; //Shadow Map
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[12].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[12]; //cbFrameInfo
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	// -> �ϴ� �ӽ÷� ALL(TrackingTime)

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[13]; //pattern Texture
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[14]; //cbFrameworkInfo
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[15]; //RandomTexture
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[16]; //RWTexture2D
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[17].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[17].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[17]; //RWTexture2D
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDescs[3];

	d3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].MipLODBias = 0;
	d3dSamplerDescs[0].MaxAnisotropy = 1;
	d3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[0].MinLOD = 0;
	d3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[0].ShaderRegister = 0;
	d3dSamplerDescs[0].RegisterSpace = 0;
	d3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	d3dSamplerDescs[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	d3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	d3dSamplerDescs[1].MipLODBias = 0.0f;
	d3dSamplerDescs[1].MaxAnisotropy = 1;
	d3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	d3dSamplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	d3dSamplerDescs[1].MinLOD = 0;
	d3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[1].ShaderRegister = 2;
	d3dSamplerDescs[1].RegisterSpace = 0;
	d3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDescs[2].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	d3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[2].MipLODBias = 0;
	d3dSamplerDescs[2].MaxAnisotropy = 1;
	d3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[2].MinLOD = 0;
	d3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[2].ShaderRegister = 3;
	d3dSamplerDescs[2].RegisterSpace = 0;
	d3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(d3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)
		&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) {
		pd3dSignatureBlob->Release();
	}
	if (pd3dErrorBlob) {
		pd3dErrorBlob->Release();
	}
}

int CScene::m_nCntCbv = 0;
int CScene::m_nCntSrv = 0;
int CScene::m_nCntUav = 0;

void CScene::CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	if (m_pd3dCbvSrvUavDescriptorHeap.Get())
	{
		m_pd3dCbvSrvUavDescriptorHeap.Reset();
	}

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews  + nUnorderedAccessViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT h  = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvUavDescriptorHeap);

	//m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	//m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dUavCPUDescriptorStartHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);
	m_d3dUavGPUDescriptorStartHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle;
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle;
	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
	m_d3dUavCPUDescriptorNextHandle = m_d3dUavCPUDescriptorStartHandle;
	m_d3dUavGPUDescriptorNextHandle = m_d3dUavGPUDescriptorStartHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	m_nCntCbv++;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvUavDescriptorIncrementSize;
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvUavDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, const shared_ptr<CTexture>& pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_nCntSrv++;
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			pTexture->SetSrvGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	d3dShaderResourceViewDesc.Format = dxgiSrvFormat;
	d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
	d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
	m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
	m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

	return(d3dSrvGPUDescriptorHandle);
}

// �ϴ��� ������ Uav�ϳ��� ������ �Ǵ� �״�� ����غ���
void CScene::CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, const shared_ptr<CTexture>& pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_nCntUav++;
	
	m_d3dUavCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dUavGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	
	if (pTexture) 
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = pTexture->GetUnorderedAccessViewDesc(i);
			pd3dDevice->CreateUnorderedAccessView(pShaderResource, NULL, &d3dUnorderedAccessViewDesc, m_d3dUavCPUDescriptorNextHandle);
			m_d3dUavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			pTexture->SetUavGpuDescriptorHandle(i, m_d3dUavGPUDescriptorNextHandle);
			m_d3dUavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
	
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
}

/// <CScene>
/////////////////////////////////////////////////////////////////////
/// <CScene - CLobbyScene>

CLobbyScene::CLobbyScene(HWND hWnd, weak_ptr<CCamera>& pCamera)
{
	m_hWnd = hWnd;

	m_pCamera = make_shared<CCamera>();
	m_pCamera->GenerateProjectionMatrix(0.01f, 50.0f, ASPECT_RATIO, 45);
	m_pCamera->RegenerateViewMatrix();
	pCamera = m_pCamera;
}

bool CLobbyScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) 
{ 
	shared_ptr<CLobbyUserInterfaceShader> pLobbyUIShader = dynamic_pointer_cast<CLobbyUserInterfaceShader>(m_vpShader[LOBBY_UI_SHADER]);
	POINT ptCursorPos = m_ptCursor;
	ptCursorPos.y = FRAME_BUFFER_HEIGHT - ptCursorPos.y;
	switch (nMessageID)
	{
	case WM_MOUSEMOVE:
		m_ptCursor.x = ((int)(short)LOWORD(lParam));
		m_ptCursor.y = ((int)(short)HIWORD(lParam));

		break;
	case WM_LBUTTONDOWN:
	{
		ProcessButtonDown(ptCursorPos, pLobbyUIShader);
		ProcessClickBorder(ptCursorPos, pLobbyUIShader);
		// ���õ� ���� ��ȣ
		m_nSelectedSlot = pLobbyUIShader->GetSelectedBorder();
	}
		break;
	case WM_LBUTTONUP:
	{
		int nRetVal = pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_UP);
		if (nRetVal == 1)	// GAME START
		{
			PostMessage(m_hWnd, WM_SOCKET, NULL, MAKELPARAM(FD_WRITE, 0));
			break;
		}
		nRetVal = pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_UP);
		if (nRetVal == 2)
		{
			// CHANGE 
			PostMessage(hWnd, WM_CHANGE_SLOT, 0, 0);
			break;
		}
	}
	break;
	}
	return false;
}

void CLobbyScene::ProcessButtonDown(const POINT& ptCursorPos, std::shared_ptr<CLobbyUserInterfaceShader>& pLobbyUIShader)
{
	float fWidth, fHeight;
	float fCenterX, fCenterY;
	fCenterX = FRAME_BUFFER_WIDTH * 0.75f;
	fCenterY = FRAME_BUFFER_HEIGHT * 0.2f;
	fWidth = FRAME_BUFFER_WIDTH / 2.0f * 0.5f;

	float fButtonScale = 160.0f / 680.0f;
	fHeight = FRAME_BUFFER_HEIGHT / 2.0f * fButtonScale;


	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_DOWN);
	}

	fCenterX = FRAME_BUFFER_WIDTH * 0.25f;
	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_DOWN);
	}
}

void CLobbyScene::ProcessClickBorder(const POINT& ptCursorPos, std::shared_ptr<CLobbyUserInterfaceShader>& pLobbyUIShader)
{
	// BORDER
	float fWidth, fHeight;
	float fCenterX, fCenterY;
	float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		fCenterX = FRAME_BUFFER_WIDTH * 0.116f + FRAME_BUFFER_WIDTH * 0.192f * i;
		fCenterX = FRAME_BUFFER_WIDTH * 0.116f + FRAME_BUFFER_WIDTH / 2.0f * fxScale * 0.6f * i;
		fCenterY = FRAME_BUFFER_HEIGHT * 0.65f;
		fWidth = FRAME_BUFFER_WIDTH / 2.0f * fxScale * 0.6f;
		fHeight = FRAME_BUFFER_HEIGHT / 2.0f;
		if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
		{
			pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::BORDER_SEL + i);
		}
	}
}

bool CLobbyScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//switch (nMessageID)
	//{
	//case WM_KEYDOWN:
	//	switch (wParam)
	//	{
	//	case VK_PRIOR:
	//	{
	//		XMFLOAT3 xmf3Up = XMFLOAT3(0.0f, 0.1f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Up, false);
	//	}
	//		break;
	//	case VK_NEXT:
	//	{
	//		XMFLOAT3 xmf3Down = XMFLOAT3(0.0f, -0.1f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Down, false);
	//	}
	//		break;
	//	case VK_UP:
	//	{
	//		XMFLOAT3 xmf3Back = XMFLOAT3(0.0f, 0.0f, 0.1f);
	//		m_apPlayer[0]->Move(xmf3Back, false);
	//	}
	//		break;
	//	case VK_DOWN:
	//	{
	//		XMFLOAT3 xmf3Forward = XMFLOAT3(0.0f, 0.0f, -0.1f);
	//		m_apPlayer[0]->Move(xmf3Forward, false);
	//	}
	//		break;
	//	case VK_RIGHT:
	//	{	
	//		XMFLOAT3 xmf3Right = XMFLOAT3(0.1f, 0.0f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Right, false); 
	//	}
	//		break;
	//	case VK_LEFT:
	//	{
	//		XMFLOAT3 xmf3Left = XMFLOAT3(-0.1f, 0.0f, 0.0f);
	//		m_apPlayer[0]->Move(xmf3Left, false); 
	//	}
	//		break;
	//	}
	//	break;
	//}

	return true;
}

void CLobbyScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId)
{
	CreateGraphicsRootSignature(pd3dDevice);

	int nCntCbv = 3000;
	int nCntSrv = 1000;
	int nCntUav = 1;

	CreateCbvSrvUavDescriptorHeaps(pd3dDevice, nCntCbv, nCntSrv, nCntUav);

	// ���̴� vector�� ������ ������� �ε��� define�� ������ ����
	m_vpShader.push_back(make_shared<CLobbyStandardShader>());
	m_vpShader[LOBBY_SATANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
	
	m_vpShader.push_back(make_shared<CLobbyUserInterfaceShader>(mainPlayerId));
	m_vpShader[LOBBY_UI_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	//Player ���� + ������
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (i == ZOMBIEPLAYER)
		{
			m_apPlayer[i] = std::make_shared<CZombiePlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pZombiePlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/Zom_1.bin", MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pZombiePlayerModel);
			m_vpShader[LOBBY_SATANDARD_SHADER]->AddGameObject(m_apPlayer[i]);

			float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
			XMFLOAT3 xmf3Position = XMFLOAT3(-fxScale * 1.2f + fxScale * 0.6f * i, -0.1f, 0.5f);
			XMFLOAT3 xmf3Scale = XMFLOAT3(0.3f, 0.5f, 0.3f);
			m_apPlayer[i]->SetWorldPostion(xmf3Position);
			m_apPlayer[i]->SetScale(xmf3Scale);
			m_apPlayer[i]->Rotate(0.0f, 135.0f, 0.0f);
			m_apPlayer[i]->OnUpdateToParent();
		}
		else
		{
			m_apPlayer[i] = std::make_shared<CBlueSuitPlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pBlueSuitPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/BlueSuitFree01.bin", MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pBlueSuitPlayerModel);
			m_vpShader[LOBBY_SATANDARD_SHADER]->AddGameObject(m_apPlayer[i]);

			float fxScale = float(FRAME_BUFFER_HEIGHT) / FRAME_BUFFER_WIDTH;
			XMFLOAT3 xmf3Position = XMFLOAT3(-fxScale * 1.2f + fxScale * 0.6f * i, -0.1f, 0.5f);
			XMFLOAT3 xmf3Scale = XMFLOAT3(0.35f, 0.35f, 0.35f);
			m_apPlayer[i]->SetWorldPostion(xmf3Position);
			m_apPlayer[i]->SetScale(xmf3Scale);
			m_apPlayer[i]->Rotate(0.0f, -165.0f, .0f);
			m_apPlayer[i]->OnUpdateToParent();
		}
	}

	m_vpShader[LOBBY_UI_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	m_vFullScreenProcessingShader = make_unique<CFullScreenProcessingShader>();
	m_vFullScreenProcessingShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
	m_vFullScreenProcessingShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), L"Lobby", m_apPlayer[mainPlayerId]);
}

void CLobbyScene::AnimateObjects(float fElapsedTime, float fCurTime)
{
	for (auto& pShader : m_vpShader)
	{
		pShader->AnimateObjects(fElapsedTime);
	}
}

bool CLobbyScene::ProcessInput(UCHAR* pKeysBuffer) 
{
	shared_ptr<CLobbyUserInterfaceShader> pLobbyUIShader = dynamic_pointer_cast<CLobbyUserInterfaceShader>(m_vpShader[LOBBY_UI_SHADER]);

	POINT ptCursorPos = m_ptCursor;
	ptCursorPos.y = FRAME_BUFFER_HEIGHT - ptCursorPos.y;

	// START BUTTON
	float fWidth, fHeight;
	float fCenterX, fCenterY;
	fCenterX = FRAME_BUFFER_WIDTH * 0.75f;
	fCenterY = FRAME_BUFFER_HEIGHT * 0.2f;
	fWidth = FRAME_BUFFER_WIDTH / 2.0f * 0.5f;

	float fButtonScale = 160.0f / 680.0f;
	fHeight = FRAME_BUFFER_HEIGHT / 2.0f * fButtonScale;

	
	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_SEL);
	}
	else
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::START_BUTTON_NON);
	}

	fCenterX = FRAME_BUFFER_WIDTH * 0.25f;
	if (CheckCursor(ptCursorPos, fCenterX, fCenterY, fWidth, fHeight))
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_SEL);
	}
	else
	{
		pLobbyUIShader->ProcessInput(LOBBY_PROCESS_INPUT::CHANGE_BUTTON_NON);
	}

	return false; 
}

void CLobbyScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());
	if (m_pd3dCbvSrvUavDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, m_pd3dCbvSrvUavDescriptorHeap.GetAddressOf());

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	//UpdateShaderVariables(pd3dCommandList);
}

void CLobbyScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	PrepareRender(pd3dCommandList, pCamera);

	//m_vpShader[LOBBY_UI_SHADER]->Render(pd3dCommandList, pCamera, m_pMainPlayer, 0);
	for (auto& pShader : m_vpShader)
	{
		pShader->Render(pd3dCommandList, pCamera, m_pMainPlayer, 0);
	}
}

void CLobbyScene::LoadingRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	PrepareRender(pd3dCommandList, m_pCamera);

	m_vFullScreenProcessingShader->Render(pd3dCommandList, m_pCamera, m_pMainPlayer);
}

bool CLobbyScene::CheckCursor(POINT ptCursor, float fCenterX, float fCenterY, float fWidth, float fHeight)
{
	if (ptCursor.x > fCenterX - fWidth / 2 && ptCursor.x < fCenterX + fWidth / 2 &&
		ptCursor.y > fCenterY - fHeight / 2 && ptCursor.y < fCenterY + fHeight / 2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CLobbyScene::UpdateShaderMainPlayer(int nMainClientId) 
{
	shared_ptr<CLobbyUserInterfaceShader> pLobbyUIShader = dynamic_pointer_cast<CLobbyUserInterfaceShader>(m_vpShader[LOBBY_UI_SHADER]);
	pLobbyUIShader->UpdateShaderMainPlayer(nMainClientId);

}

/// <CScene - CLobbyScene>
/////////////////////////////////////////////////////////////////////
/// <CScene - CMainScene>

CMainScene::CMainScene()
{

}
CMainScene::~CMainScene()
{

}

void CMainScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int mainPlayerId)
{
	CreateGraphicsRootSignature(pd3dDevice);

	// ���̴� vector�� ������ ������� �ε��� define�� ������ ����
	m_vShader.push_back(make_unique<StandardShader>());
	//DXGI_FORMAT pdxgiRtvFormats[4] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };
	// Color, zDepth, Position, Emissive, Normal, HDR LOW, HDR High
	DXGI_FORMAT pdxgiRtvFormats[ADD_RENDERTARGET_COUNT] = { DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM,  DXGI_FORMAT_R8G8B8A8_UNORM };
	
	DXGI_FORMAT pdxgiRtvShadowFormat = DXGI_FORMAT_R32_FLOAT;
	m_vShader[STANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), ADD_RENDERTARGET_COUNT, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vShader.push_back(make_unique<InstanceStandardShader>());
	m_vShader[INSTANCE_STANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), ADD_RENDERTARGET_COUNT, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vShader.push_back(make_unique< CSkinnedAnimationStandardShader>());
	m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), ADD_RENDERTARGET_COUNT, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vForwardRenderShader.push_back(make_unique<TransparentShader>());
	m_vForwardRenderShader[TRANSPARENT_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vForwardRenderShader.push_back(make_unique<ParticleShader>());
	m_vForwardRenderShader[PARTICLE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_vForwardRenderShader[PARTICLE_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	m_vForwardRenderShader.push_back(make_unique<TextureBlendAnimationShader>());
	m_vForwardRenderShader[TEXTUREBLEND_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	
	m_vForwardRenderShader.push_back(make_unique<CTrailShader>());
	m_vForwardRenderShader[TRAIL_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_vForwardRenderShader[TRAIL_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	//[0505] UI
	if (mainPlayerId == ZOMBIEPLAYER)
	{
		m_vForwardRenderShader.push_back(make_unique<CZombieUserInterfaceShader>());
		m_vForwardRenderShader[USER_INTERFACE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}
	else
	{
		m_vForwardRenderShader.push_back(make_unique<CBlueSuitUserInterfaceShader>());
		m_vForwardRenderShader[USER_INTERFACE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	m_vPreRenderShader.push_back(make_unique<PartitionInsStandardShader>());
	m_vPreRenderShader[PARTITION_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 4, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);

	LoadScene(pd3dDevice, pd3dCommandList);

	// [0523] ���� �÷��̾ �ƴϾ OutLineShader�� ������ ����
	m_vForwardRenderShader.push_back(make_unique<COutLineShader>(mainPlayerId));
	m_vForwardRenderShader[OUT_LINE_SHADER]->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);

	//Player ���� + ������
	for (int i = 0; i < MAX_CLIENT; ++i)
	{
		if (i == ZOMBIEPLAYER)
		{
			m_apPlayer[i] = std::make_shared<CZombiePlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pZombiePlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/Zom_1.bin", MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pZombiePlayerModel);
			m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(m_apPlayer[i]);
			
			auto zombiePlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayer[i]);
			zombiePlayer->SetAttackTrail(dynamic_cast<CTrailShader*>(m_vForwardRenderShader[TRAIL_SHADER].get())->GetZombieSwordTrail1());
			zombiePlayer->SetAttackTrail(dynamic_cast<CTrailShader*>(m_vForwardRenderShader[TRAIL_SHADER].get())->GetZombieSwordTrail2());
			// [0506] OutLine Shader
			if (mainPlayerId == ZOMBIEPLAYER)
			{
				// ZOMBIE PLAYER�� ��� ZOMBIE �߰�
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
			}

			continue;
		}
		else
		{
			m_apPlayer[i] = std::make_shared<CBlueSuitPlayer>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), nullptr);
			shared_ptr<CLoadedModelInfo> pBlueSuitPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/BlueSuitFree01.bin",MeshType::Standard);
			m_apPlayer[i]->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pBlueSuitPlayerModel);
			m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->AddGameObject(m_apPlayer[i]);

			//[0505] BLUE SUIT �÷��̾��� �ܰ����� �׸��� ����
			// Zombie �÷��̾�߸� �ʿ��ϴ�
			if(mainPlayerId == ZOMBIEPLAYER)
			{
				// ZOMBIE PLAYER�� ��� ������ �÷��̾� �߰�
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
			}
			else if (mainPlayerId == i)
			{
				// ZOMBIE PLAYER�� �ƴ� ��� �ڽ��� MainPlayer ����
				m_vForwardRenderShader[OUT_LINE_SHADER]->AddGameObject(m_apPlayer[i]);
				auto pBSPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
				auto pZombie = dynamic_pointer_cast<CZombiePlayer>(m_apPlayer[ZOMBIEPLAYER]);
				pBSPlayer->SetZombiePlayer(pZombie);
			}
		}

		//�÷��ö���Ʈ�� �ε�
		shared_ptr<CTeleportObject> flashLight = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pflashLightModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Flashlight.bin", MeshType::Standard);
		flashLight->ObjectCopy(pd3dDevice, pd3dCommandList, pflashLightModel->m_pModelRootObject);
		//flashLight->LoadModelAndAnimation(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), pflashLightModel);
		m_vShader[STANDARD_SHADER]->AddGameObject(flashLight);

		//���̴��� �ε�
		shared_ptr<CRadarObject> pRaderObject = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pRaderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Radar.bin", MeshType::Standard);
		pRaderObject->ObjectCopy(pd3dDevice, pd3dCommandList, pRaderModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pRaderObject);

		shared_ptr<CTeleportObject> pTeleportObject = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pTeleportItemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/TeleportItem.bin", MeshType::Standard);
		pTeleportObject->ObjectCopy(pd3dDevice, pd3dCommandList, pTeleportItemModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pTeleportObject);

		shared_ptr<CMineObject> pMineObject = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pMineItemModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Item_Mine.bin", MeshType::Standard);
		pMineObject->ObjectCopy(pd3dDevice, pd3dCommandList, pMineItemModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pMineObject);

		shared_ptr<CFuseObject> pFuseObject = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pFuseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/fuse_hi-obj.bin", MeshType::Standard);
		pFuseObject->ObjectCopy(pd3dDevice, pd3dCommandList, pFuseModel->m_pModelRootObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pFuseObject);

		auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
		if (player) {
			player->SetFlashLight(flashLight);
			player->SetRader(pRaderObject);
			player->SetTeleportItem(pTeleportObject);
			player->SetMineItem(pMineObject);
			player->SetFuseItem(pFuseObject);
		}
	}
	
	/*auto surviveMainPlayer = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[mainPlayerId]);
	auto zombieMainPlayer = dynamic_pointer_cast<CZombiePlayer>(m_apPlayer[mainPlayerId]);*/
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// ������
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	// ������ ������ ���������� �����غ����ҵ�? �ϴ� �������� ����ġ�� ����
	for (int i = 0; i < 9; ++i) // �����۵� �ν��Ͻ� ó���� �ؾ���.���� ��������
	{
		shared_ptr<CFuseObject> pFuseObject = make_shared<CFuseObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pFuseModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/fuse_hi-obj.bin", MeshType::Standard);
		pFuseObject->ObjectCopy(pd3dDevice, pd3dCommandList, pFuseModel->m_pModelRootObject);
		
		g_collisionManager.AddCollisionObject(pFuseObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pFuseObject);
	}

	for (int i = 0; i < 15; ++i)
	{
		shared_ptr<CTeleportObject> pTeleportObject = make_shared<CTeleportObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pTeleportModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/TeleportItem.bin", MeshType::Standard);
		pTeleportObject->ObjectCopy(pd3dDevice, pd3dCommandList, pTeleportModel->m_pModelRootObject);
		
		g_collisionManager.AddCollisionObject(pTeleportObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pTeleportObject);
	}

	for (int i = 0; i < 2; ++i)
	{
		//���̴��� �ε�
		shared_ptr<CRadarObject> pRaderObject = make_shared<CRadarObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		static shared_ptr<CLoadedModelInfo> pRaderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Radar.bin", MeshType::Standard);
		pRaderObject->ObjectCopy(pd3dDevice, pd3dCommandList, pRaderModel->m_pModelRootObject);

		g_collisionManager.AddCollisionObject(pRaderObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pRaderObject);
	}

	shared_ptr<CLoadedModelInfo> pElectricBlendModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), "Asset/Model/electricBlend.bin", MeshType::Blend);
	shared_ptr<CLoadedModelInfo> pMineModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), (char*)"Asset/Model/Item_Mine.bin", MeshType::Standard);
	for (int i = 0; i < 50; ++i)
	{	//CJI [0422] : ���ھ����� ����Ʈ�� �����Ҵ��� ���̱����ؼ� �̸� ������ ���� ��ü�� �̿��� �������Ѵ�.
		shared_ptr<TextureBlendObject> mineExplosionObject = make_shared<TextureBlendObject>(pd3dDevice, pd3dCommandList, pElectricBlendModel->m_pModelRootObject, m_apPlayer[mainPlayerId]);
		m_vTextureBlendObjects.push_back(mineExplosionObject);
		m_vForwardRenderShader[TEXTUREBLEND_SHADER]->AddGameObject(m_vTextureBlendObjects[i]);

		shared_ptr<CMineObject> pMineObject = make_shared<CMineObject>(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
		pMineObject->SetExplosionObject(mineExplosionObject);
		pMineObject->ObjectCopy(pd3dDevice, pd3dCommandList, pMineModel->m_pModelRootObject);

		g_collisionManager.AddCollisionObject(pMineObject);
		m_vShader[STANDARD_SHADER]->AddGameObject(pMineObject);
		for (int j = 0; j < MAX_CLIENT; ++j) {
			auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[j]);
			if (player) {
				player->AddEnvironmentMineItems(pMineObject);
			}
		}
	}
	
	// [0504] UserInterface
	m_vForwardRenderShader[USER_INTERFACE_SHADER]->AddGameObject(m_apPlayer[mainPlayerId]);
	m_vForwardRenderShader[USER_INTERFACE_SHADER]->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());

	BuildLights(pd3dDevice, pd3dCommandList); // ���̴� ���� ���� �����ϵ��� �Ѵ�.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//[0626] gameFramework���� �̵�
	INT ncbElementBytes = ((sizeof(FrameTimeInfo) + 255) & ~255); //256�� ���

	m_pd3dcbTime = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbTime->Map(0, NULL, (void**)&m_pcbMappedTime);
	m_pcbMappedTime->gfScale = 2.0f;
	m_pcbMappedTime->gfBias = -0.1f;
	m_pcbMappedTime->gfIntesity = 1.5f;
	m_d3dTimeCbvGPUDescriptorHandle = CScene::CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbTime.Get(), ncbElementBytes);

	//[0626] ����Ʈ ���μ��� ���̴��� Scene���� ���鼭 �ű�
	m_pPostProcessingShader = new CPostProcessingShader();
	m_pPostProcessingShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_d3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);

	DXGI_FORMAT pdxgiResourceFormats[ADD_RENDERTARGET_COUNT] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM,	DXGI_FORMAT_R8G8B8A8_UNORM };

	m_pPostProcessingShader->CreateResourcesAndRtvsSrvs(pd3dDevice, pd3dCommandList, ADD_RENDERTARGET_COUNT, pdxgiResourceFormats, d3dRtvCPUDescriptorHandle); //SRV to (Render Targets) + (Depth Buffer) + EMISSIVE 

	d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * ADD_RENDERTARGET_COUNT);
	m_pPostProcessingShader->CreateShadowMapResource(pd3dDevice, pd3dCommandList, m_nLights, d3dRtvCPUDescriptorHandle);

	//[0523] ���� ���� �÷��̾� �ܿ��� ���, COutLineShader ���ο��� m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0)�� ����ϱ����ؼ� �ʿ�
	dynamic_cast<COutLineShader*>(m_vForwardRenderShader[OUT_LINE_SHADER].get())->SetPostProcessingShader(m_pPostProcessingShader);

	//��ǻƮ ���̴�
	m_pBlurComputeShader = make_shared<CBlurComputeShader>();
	m_pBlurComputeShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
	m_pBlurComputeShader->SetTextureRtv(m_pPostProcessingShader->GetTexture());

	//[0626] 

	//[0721] FullScreen
	m_vFullScreenProcessingShader = make_unique<CFullScreenProcessingShader>();
	m_vFullScreenProcessingShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get());
	m_vFullScreenProcessingShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), L"Main", m_apPlayer[mainPlayerId]);

	m_pTextureToScreenShaderShader = make_shared<CTextureToScreenShader>(m_pBlurComputeShader->GetTextureSrv());
	//m_pTextureToScreenShaderShader = make_shared<CTextureToScreenShader>();
	m_pTextureToScreenShaderShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), 1, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
}


void CMainScene::AddDefaultObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ObjectType type, XMFLOAT3 position, int shader, int mesh)
{
	shared_ptr<CGameObject> pObject;
	switch (type)
	{
	case ObjectType::DEFAULT:
		pObject = make_shared<CGameObject>(pd3dDevice, pd3dCommandList, 1);
		break;
	case ObjectType::HEXAHERON:
		pObject = make_shared<CHexahedronObject>(pd3dDevice, pd3dCommandList, 1);
		break;
	default:
		break;
	}
	pObject->SetMesh(m_vMesh[mesh]);
	pObject->SetPosition(position);

	m_vShader[shader]->AddGameObject(pObject);
}

void CMainScene::BuildLights(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_nLights = ReadLightObjectInfo(m_xmf3lightPositions, m_xmf3lightLooks) + MAX_SURVIVOR/*�÷��̾� ����*/;
	if (m_nLights > MAX_LIGHTS) {
		//m_nLights = MAX_LIGHTS;
	}
	m_pLights = new LIGHT[MAX_LIGHTS];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * MAX_LIGHTS);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	for (int i = 0; i < MAX_SURVIVOR;++i) {
		m_pLightCamera.push_back(make_shared<CLightCamera>());
		m_pLightCamera[i]->m_pLight = make_shared<LIGHT>();

		m_xmf3lightPositions.insert(m_xmf3lightPositions.begin(), XMFLOAT3(0.0f, -100.0f, 0.0f)); // m_xmf3lightPositions�� ������ ī�޶� �������
		m_xmf3lightLooks.insert(m_xmf3lightLooks.begin(), XMFLOAT3(0.0f, -1.0f, 0.0f));

		m_pLightCamera[i]->m_pLight->m_bEnable = true;
		m_pLightCamera[i]->m_pLight->m_nType = SPOT_LIGHT;
		m_pLightCamera[i]->m_pLight->m_fRange = 30.0f;
		m_pLightCamera[i]->m_pLight->m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Attenuation = XMFLOAT3(1.0f, -0.1f, 0.01f);
		m_pLightCamera[i]->m_pLight->m_fFalloff = 1.0f;
		m_pLightCamera[i]->m_pLight->m_fPhi = (float)cos(XMConvertToRadians(35.0f));
		m_pLightCamera[i]->m_pLight->m_fTheta = (float)cos(XMConvertToRadians(25.0f));
	}
	m_pLights[0].m_bEnable = true;

	for (int i = MAX_SURVIVOR; i < m_nLights;++i) {
		m_pLightCamera.push_back(make_shared<CLightCamera>());
		m_pLightCamera[i]->m_pLight = make_shared<LIGHT>();

		m_pLightCamera[i]->m_pLight->m_bEnable = true;
		m_pLightCamera[i]->m_pLight->m_nType = SPOT_LIGHT;
		m_pLightCamera[i]->m_pLight->m_fRange = 30.0f;
		m_pLightCamera[i]->m_pLight->m_xmf4Ambient = XMFLOAT4(0.6f, 0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Diffuse = XMFLOAT4(0.6f, 0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf4Specular = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		m_pLightCamera[i]->m_pLight->m_xmf3Position = m_xmf3lightPositions[i];
		m_pLightCamera[i]->m_pLight->m_xmf3Direction = m_xmf3lightLooks[i];
		m_pLightCamera[i]->m_pLight->m_xmf3Attenuation = XMFLOAT3(1.0f, -0.1f, 0.01f);

		m_pLightCamera[i]->m_pLight->m_fFalloff = 1.0f;
		m_pLightCamera[i]->m_pLight->m_fPhi = (float)cos(XMConvertToRadians(45.0f));
		m_pLightCamera[i]->m_pLight->m_fTheta = (float)cos(XMConvertToRadians(35.0f));
	}

	

	vector<XMFLOAT3> positions = GetLightPositions();
	vector<XMFLOAT3> looks = GetLightLooks();

	XMFLOAT3 xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);

	XMFLOAT4X4 xmf4x4ToTexture = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	XMMATRIX xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);
	XMMATRIX xmmtxViewProjection;

	for (int i = 0;i < m_nLights;++i) {

		XMFLOAT3 xmf3Up = Vector3::CrossProduct(looks[i], xmf3Right);
		XMFLOAT3 lookAtPosition = Vector3::Add(positions[i], looks[i]);
		m_pLightCamera[i]->GenerateViewMatrix(positions[i], lookAtPosition, xmf3Up);
		if (i >= MAX_SURVIVOR)
		{
			m_pLightCamera[i]->GenerateProjectionMatrix(1.01f, 5.0f, ASPECT_RATIO, 90.0f);	//[0513] ������� �־��  �׸��ڸ� �׸�
		}/*
		else {
			m_pLightCamera[i]->GenerateProjectionMatrix(0.01f, 5.0f, ASPECT_RATIO, 90.0f);
		}*/
		m_pLightCamera[i]->GenerateFrustum();
		m_pLightCamera[i]->MultiplyViewProjection();

		XMFLOAT4X4 viewProjection = m_pLightCamera[i]->GetViewProjection();
		xmmtxViewProjection = XMLoadFloat4x4(&viewProjection);
		XMStoreFloat4x4(&m_pLightCamera[i]->m_pLight->m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));
		m_pLightCamera[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}

	// ���� ī�޶� ��Ƽ�� ����
	// ��Ƽ���� ���庯ȯ����� 0,0,0���� �����϶�.
	// �ٿ�� �ڽ��� ��ġ�� �����Ѱ��̴�.
	unique_ptr<PartitionInsStandardShader> PtShader(static_cast<PartitionInsStandardShader*>(m_vPreRenderShader[PARTITION_SHADER].release()));
	auto vBB = PtShader->GetPartitionBB();

	for (int i = 0; i < m_nLights;++i) {
		BoundingBox camerabb;
		camerabb.Center = m_pLightCamera[i]->GetPosition();
		camerabb.Extents = XMFLOAT3(0.1f, 0.1f, 0.1f);
		int curFloor = static_cast<int>(std::floor(camerabb.Center.y / 4.5f));

		m_pLightCamera[i]->SetFloor(curFloor);
		for (int bbIdx = 0; bbIdx < vBB.size();++bbIdx) {
			if (vBB[bbIdx]->Intersects(camerabb)) {
				m_pLightCamera[i]->SetPartition(bbIdx);
				break;
			}
		}
	}

	m_vPreRenderShader[PARTITION_SHADER].reset(PtShader.release());
}

void CMainScene::LoadScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	ifstream tpobFile("Asset/Data/����ü.txt");
	// �̸� , �������� , �����ε���
	if (!tpobFile) {
		assert(0);
	}
	unordered_map<string, vector<int>> transparentObjects;

	string name;
	while (tpobFile >> name)
	{
		int count{};
		tpobFile >> count;
		for (int i = 0; i < count; ++i) {
			int mtNum;
			tpobFile >> mtNum;
			transparentObjects[name].push_back(mtNum);
		}
	}

	FILE* pSceneFile = NULL;
	::fopen_s(&pSceneFile, (char*)"Asset/Model/Scene.bin", "rb");
	::rewind(pSceneFile);
	int fileEnd{};
	
	unique_ptr<InstanceStandardShader> InsStShader(static_cast<InstanceStandardShader*>(m_vShader[INSTANCE_STANDARD_SHADER].release()));
	int n_curfloor = -1;
	static int count{};
	while (true)
	{
		shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();

		char pstrToken[128] = { '\0' };
		for (; ; )
		{
			if (::ReadStringFromFile(pSceneFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Floor>:")) {
					InsStShader->m_vFloorObjects.push_back(vector<shared_ptr<CGameObject>>());
					n_curfloor += 1;
				}
				else if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					count++;
					pLoadedModel->m_pModelRootObject = CGameObject::LoadInstanceFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), NULL, pSceneFile, &pLoadedModel->m_nSkinnedMeshes);
					::ReadStringFromFile(pSceneFile, pstrToken); //"</Hierarchy>"
					pLoadedModel->m_pModelRootObject->Rotate(0.0f, 0.0f, 0.0f);

					//if (!pLoadedModel->m_pModelRootObject->m_pChild->m_pMesh) continue;

					if (!transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName].empty()) {
						pLoadedModel->m_pModelRootObject->SetTransparentObjectInfo(transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName]);
						InsStShader->m_vFloorObjects[n_curfloor].push_back(pLoadedModel->m_pModelRootObject);
						m_vForwardRenderShader[TRANSPARENT_SHADER]->AddGameObject((pLoadedModel->m_pModelRootObject));
						// ù��° ���̴��� �������� �����鸸 ������, �ι�° ���̴��� ������ �����鸸 ������ �з��� �����̰� �������� �������ؾ��ϱ� ������ �� ���̴��� ��� �����Ѵ�. 
					}
					else
					{
						InsStShader->m_vFloorObjects[n_curfloor].push_back(pLoadedModel->m_pModelRootObject);
					}
				}
				else if (!strcmp(pstrToken, "<Animation>:"))
				{
					CGameObject::LoadAnimationFromFile(pSceneFile, pLoadedModel);
					pLoadedModel->PrepareSkinning();
				}
				else if (!strcmp(pstrToken, "</Animation>:"))
				{
					break;
				}
				else if (!strcmp(pstrToken, "</Scene>:"))
				{
					fileEnd = 1;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (fileEnd) {
			break;
		}
	}

	for (int i = 0; i < g_collisionManager.GetNumOfCollisionObject();++i) { // ���庯ȯ ����� ������ �ִ� ��ü��.
		auto pObject = g_collisionManager.GetCollisionObjectWithNumber(i).lock();
		if (!transparentObjects[pObject->m_pstrFrameName].empty()) {
			pObject->SetTransparentObjectInfo(transparentObjects[pObject->m_pstrFrameName]);
		}
	}

	for (const auto& floorObjects : InsStShader->m_vFloorObjects) {
		for (const auto& ob : floorObjects) {
			if (!strcmp(ob->m_pstrFrameName, "Ins_Biological_Capsule_1")) {
				if (!ob->m_pChild) {
					continue;
				}

				auto ins_obs = dynamic_pointer_cast<CInstanceObject>(ob->m_pChild);
				for (const auto& ins_ob : ins_obs->m_vInstanceObjectInfo) {
					XMFLOAT3 pos = ins_ob->GetPosition();
					pos.y += 0.6f;
					sharedobject.m_vParticleObjects[CParticleMesh::BUBBLE]->SetParticleInsEnable(-1, true, 0.0f, pos);
				}
			}
		}
	}


	// ��Ƽ�� ������ �� �ε�
	FILE* pPartitionFile = NULL;
	::fopen_s(&pPartitionFile, (char*)"Asset/Model/PartisionScene.bin", "rb");
	::rewind(pPartitionFile);
	fileEnd = 0;

	unique_ptr<PartitionInsStandardShader> PtShader(static_cast<PartitionInsStandardShader*>(m_vPreRenderShader[PARTITION_SHADER].release()));
	int nPartition = -1;
	g_InstanceMeshNotAddCollision = true; // �� ������Ʈ���� Collision üũ�� �� �ʿ� ���� ��ü���̴�.(�ܼ��� ��������� ����°����� ���� ��ü��)
	while (true)
	{
		shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();

		char pstrToken[128] = { '\0' };

		for (; ; )
		{
			if (::ReadStringFromFile(pPartitionFile, pstrToken))
			{
				if (!strcmp(pstrToken, "<Partition>:")) {
					PtShader->AddPartition(); // ��Ƽ�� �߰�
					nPartition++;
				}
				else if (!strcmp(pstrToken, "<Bound>:")) {
					shared_ptr<BoundingBox> bb = make_shared<BoundingBox>();
					int nIndex = 0;
					XMFLOAT3 xmf3bbCenter, xmf3bbExtents;
					fread(&nIndex, sizeof(int), 1, pPartitionFile);
					(UINT)::fread(&xmf3bbCenter, sizeof(XMFLOAT3), 1, pPartitionFile);
					(UINT)::fread(&xmf3bbExtents, sizeof(XMFLOAT3), 1, pPartitionFile);

					bb->Center = xmf3bbCenter;
					bb->Extents = xmf3bbExtents;

					PtShader->AddPartitionBB(bb);
				} // �ٿ�� �ڽ��� �а� ��ü�� �ݸ��������̳ʿ��� ������ ���
				else if (!strcmp(pstrToken, "<Hierarchy>:"))
				{
					pLoadedModel->m_pModelRootObject = CGameObject::LoadInstanceFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature.Get(), NULL, pPartitionFile, &pLoadedModel->m_nSkinnedMeshes);
					::ReadStringFromFile(pPartitionFile, pstrToken); //"</Hierarchy>"
					pLoadedModel->m_pModelRootObject->Rotate(0.0f, 0.0f, 0.0f);
					if (!strcmp(pLoadedModel->m_pModelRootObject->m_pstrFrameName, "Zom_1"))
					{
					}
					else if (!transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName].empty())
					{
						//pLoadedModel->m_pModelRootObject->SetTransparentObjectInfo(transparentObjects[pLoadedModel->m_pModelRootObject->m_pstrFrameName]);
						//PtShader->AddPartitionGameObject((pLoadedModel->m_pModelRootObject), nPartition);
						// ù��° ���̴��� �������� �����鸸 ������, �ι�° ���̴��� ������ �����鸸 ������ �з��� �����̰� �������� �������ؾ��ϱ� ������ �� ���̴��� ��� �����Ѵ�. 

					}
					else
					{
						//PtShader->AddPartitionGameObject((pLoadedModel->m_pModelRootObject), nPartition);

					}
				}
				else if (!strcmp(pstrToken, "<Animation>:"))
				{
					CGameObject::LoadAnimationFromFile(pPartitionFile, pLoadedModel);
					pLoadedModel->PrepareSkinning();
				}
				else if (!strcmp(pstrToken, "</Animation>:"))
				{
					break;
				}
				else if (!strcmp(pstrToken, "</Scene>:"))
				{
					fileEnd = 1;
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (fileEnd) {
			break;
		}
	}

	//0528 CJI - ��ü�� �ٽ� ������ ���� ���� �ִ� ��ü�� �������.
	//1. ��Ƽ�ǿ� �ش��ϴ� ������Ʈ�� ��Ƽ�Ǻ��� �ִ´�.
	//2. �Ȱ��� �޽��� ������ ������Ʈ���� �ν��Ͻ�ȭ ��Ų��.
	for (int i = 0; i < g_collisionManager.GetNumOfCollisionObject();++i) { // ���庯ȯ ����� ������ �ִ� ��ü��.
		auto pObject = g_collisionManager.GetCollisionObjectWithNumber(i).lock();
		PartisionShaderCollision(PtShader, pObject);
	}
	auto nonCollisionObjects = g_collisionManager.GetNonCollisionObjects();
	for (int i = 0; i < nonCollisionObjects.size();++i) { // ���庯ȯ ����� ������ �ִ� ��ü��.
		auto pObject = nonCollisionObjects[i].lock();	
		PartisionShaderCollision(PtShader, pObject);
	}

	map<string, shared_ptr<CInstanceObject>> mStr_GameObejcts;
	auto& partition = PtShader->GetPartitionObjects();
	for (auto& objects : partition) { // �� ��Ƽ�Ǿ� ������Ʈ�� �ν��Ͻ�ȭ.
		mStr_GameObejcts.clear();
		for (auto& ob : objects) {
			if (!mStr_GameObejcts[ob->m_pstrFrameName]) {
				mStr_GameObejcts[ob->m_pstrFrameName] = make_shared<CInstanceObject>(pd3dDevice, pd3dCommandList);
			}
			mStr_GameObejcts[ob->m_pstrFrameName]->m_vInstanceObjectInfo.push_back(ob); 
			//�ν��Ͻ̵� ������Ʈ�� �޽��� �̸��� �����߱⶧���� str ������ mesh�� �̸��� �ǹ���. 
		}
		if (mStr_GameObejcts.size() == 0) continue;

		auto strVector = mStr_GameObejcts.begin();// �޽��� �̸��� �̿��� ������Ʈ�� ã�´�.
		while (strVector != mStr_GameObejcts.end())
		{
			shared_ptr<CInstanceStandardMesh> pMesh = make_shared<CInstanceStandardMesh>(pd3dDevice, pd3dCommandList);
			strncpy(pMesh->m_pstrMeshName, strVector->first.c_str(), strVector->first.size());
			CStandardMesh::SaveStandardMesh(dynamic_pointer_cast<CStandardMesh>(pMesh));

			pMesh->m_nCntInstance = strVector->second->m_vInstanceObjectInfo.size();
			XMFLOAT4X4* InsTrans = new XMFLOAT4X4[pMesh->m_nCntInstance];
			for (int i = 0; i < strVector->second->m_vInstanceObjectInfo.size();++i) {
				InsTrans[i] = Matrix4x4::Transpose(strVector->second->m_vInstanceObjectInfo[i]->m_xmf4x4World); // ���۷� ������ ����� ��ġ��ķ� ��������.
			}
			pMesh->SetInstanceTransformMatrix(InsTrans);
			pMesh->GetInstanceTransformMatrixBuffer() = ::CreateBufferResource(pd3dDevice, pd3dCommandList, InsTrans,
				sizeof(XMFLOAT4X4) * pMesh->m_nCntInstance, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
			D3D12_VERTEX_BUFFER_VIEW bufferView;
			bufferView.BufferLocation = pMesh->GetInstanceTransformMatrixBuffer()->GetGPUVirtualAddress();
			bufferView.StrideInBytes = sizeof(XMFLOAT4X4);
			bufferView.SizeInBytes = sizeof(XMFLOAT4X4) * pMesh->m_nCntInstance;
			pMesh->SetInstanceMatrixBufferView(bufferView);

			for (auto& floor : InsStShader->m_vFloorObjects) {
				bool bFind = false;
				for (auto& f_ob : floor) { // f_ob�� ���� ������Ʈ�̹Ƿ� Ins -> child.mesh�޽��� �̸����� ���ؾ���
					if (!f_ob->m_pChild || !f_ob->m_pChild->m_pMesh) continue;
					if (!strcmp(f_ob->m_pChild->m_pMesh->m_pstrMeshName, strVector->first.c_str())) {
						mStr_GameObejcts[strVector->first]->InstanceObjectCopy(pd3dDevice, pd3dCommandList, f_ob);
						bFind = true;
						break;
					}
				}
				if (bFind) break;
			}

			pMesh->SetOriginInstanceObject(dynamic_pointer_cast<CInstanceObject>(mStr_GameObejcts[strVector->first]));
			mStr_GameObejcts[strVector->first]->m_pChild->SetMesh(pMesh);// ��Ʈ ��ü�� �ν��Ͻ� �θ�ü�� �ǹ� �ϹǷ� child���� �޽��� �ο�
			if (!transparentObjects[mStr_GameObejcts[strVector->first]->m_pstrFrameName].empty())
			{
				mStr_GameObejcts[strVector->first]->SetTransparentObjectInfo(transparentObjects[mStr_GameObejcts[strVector->first]->m_pstrFrameName]);
			}
			objects.erase(
				std::remove_if(objects.begin(), objects.end(), [strVector](const shared_ptr<CGameObject>& obj) {
					return obj->m_pstrFrameName == strVector->first;
					}),
				objects.end()
			);

			++strVector;
		}
		objects.clear();
		for (auto& [meshName,insObject] : mStr_GameObejcts) {
			objects.push_back(insObject);
		}
	}
	 
	// �޸� ������ �����ϱ� ���� �ٽ� ��ȯ
	m_vShader[INSTANCE_STANDARD_SHADER].reset(InsStShader.release());
	m_vPreRenderShader[PARTITION_SHADER].reset(PtShader.release());
}

void PartisionShaderCollision(unique_ptr<PartitionInsStandardShader>& PtShader, shared_ptr<CGameObject>& pObject)
{
	if (!strcmp(pObject->m_pstrFrameName, "Wall_BoundingBox") /*|| Ins_Bounding_Stair_Start*/) {
		return;
	}

	//��ü�� �浹üũ�� �Ͽ� ���� ��Ƽ�ǿ� �ٿ���ڽ��� ������ ����
	for (auto& srcobb : pObject->GetVectorOOBB()) {
		BoundingOrientedBox dstobb;
		srcobb.Transform(dstobb, XMLoadFloat4x4(&pObject->m_xmf4x4World));
		XMStoreFloat4(&dstobb.Orientation, XMQuaternionNormalize(XMLoadFloat4(&dstobb.Orientation)));
		int nPt = 0;
		bool inPart = false;
		for (auto& ptobb : PtShader->GetPartitionBB()) {
			if (dstobb.Intersects(*ptobb)) {
				PtShader->AddPartitionGameObject(pObject, nPt);
				inPart = true;
			}
			nPt += 1;
			if (nPt >= PtShader->GetPartitionBB().size()) break;
		}

		if (inPart ) {
			break;
		}
	}
}

bool StreamReadString(ifstream& in, string& str)
{
	// ���ڿ��� ���� �б�
	char strLength;
	in.read(reinterpret_cast<char*>(&strLength), sizeof(char));

	// ���� ���� �����ϸ� ����
	if (in.eof()) return false;

	// ���ڿ� �б�
	char* buffer = new char[strLength + 1]; // ���ڿ� ���� NULL ����('\0')�� �߰��ϱ� ���� +1
	in.read(buffer, strLength);
	buffer[strLength] = '\0'; // NULL ���� �߰�
	str = buffer;
	delete[] buffer;

	//cout << str << endl;
	return true;
}

template<class T>
void StreamReadVariable(ifstream& in, T& data)
{
	in.read(reinterpret_cast<char*>(&data), sizeof(T));
}

int ReadLightObjectInfo(vector<XMFLOAT3>& positions,vector<XMFLOAT3>& looks)
{
	ifstream in("Asset/Data/LightObject.bin", ios::binary);
	if (!in.is_open()) {
		assert(0);
	}
	// ���ڿ��� ������ ����
	string str;
	XMFLOAT3 xmfloat3;
	int objCount{};

	// ���Ϸκ��� ���ڿ� �б�
	while (true) {
		if (!StreamReadString(in, str)) {
			break;
		}
		if (str == "<TotalObject>: ") {
			StreamReadVariable(in, objCount);
		}
		else if ("<GameObjects>:" == str) {
			if (!StreamReadString(in, str)) break;

			if ("<Position>:" == str) {
				StreamReadVariable(in, xmfloat3);
				positions.push_back(xmfloat3);
			}

			if (!StreamReadString(in, str)) break;
			if ("<Look>:" == str) {
				StreamReadVariable(in, xmfloat3);
				looks.push_back(xmfloat3);
			}
		}
	}
	return objCount;
}

void CMainScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256�� ���
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
	m_pcbMappedLights->bias = 0.0011f;
	m_d3dLightCbvGPUDescriptorHandle = CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbLights.Get(), ncbElementBytes);
}

void CMainScene::PrevRenderTask(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// �׸��ڸʿ� �ش��ϴ� �ؽ�ó�� ����Ÿ������ ��ȯ
	m_pPostProcessingShader->OnShadowPrepareRenderTarget(pd3dCommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	for (int i = 0; i < m_pPostProcessingShader->GetShadowTexture()->GetTextures(); ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE shadowRTVDescriptorHandle = m_pPostProcessingShader->GetShadowRtvCPUDescriptorHandle(i);
		{
			pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

			pd3dCommandList->OMSetRenderTargets(1, &shadowRTVDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

			if (i < MAX_SURVIVOR) {
				PrepareRender(pd3dCommandList, m_pLightCamera[i]);
				Render(pd3dCommandList, m_pLightCamera[i], 1/*nPipelinestate*/); // ī�޶� ���� ��ġ��� �����ؼ� ��������.
			}
			else {
				ShadowPreRender(pd3dCommandList, m_pLightCamera[i], 1/*nPipelinestate*/);
			}
		}
	}

	// �׸��ڸʿ� �ش��ϴ� ����Ÿ���� �ؽ�ó�� ��ȯ
	m_pPostProcessingShader->TransitionShadowMapRenderTargetToCommon(pd3dCommandList);
}

void CMainScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < m_nLights; ++i) {
		memcpy(&m_pLights[i], m_pLightCamera[i]->m_pLight.get(), sizeof(LIGHT));
	}

	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT)* m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dLightCbvGPUDescriptorHandle); //Lights
}

void CMainScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
	}
}

void CMainScene::ReleaseUploadBuffers()
{
	for (auto& m : m_vMesh) 
	{
		m->ReleaseUploadBuffers();
	}

	for (auto& s : m_vShader) 
	{
		s->ReleaseUploadBuffers();
	}
}

void CMainScene::ReleaseObjects()
{

}

bool CMainScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
		m_pMainPlayer->SetRightClick(true);
		m_pMainPlayer->RightClickProcess(gGameTimer.GetTotalTime());
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}

	return true;
}

bool CMainScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.1f, 0.001f); //������  ��� ��������
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'P':
			m_pcbMappedTime->gfScale += 0.1f;
			break;
		case 'O':
			m_pcbMappedTime->gfScale -= 0.1f;
			break;
		case 'L':
			m_pcbMappedTime->gfIntesity += 0.1f;
			break;
		case 'K':
			m_pcbMappedTime->gfIntesity -= 0.1f;
			break;
		case 'Y':
			m_pcbMappedTime->gfBias += 0.01f;
			break;
		case 'U':
			m_pcbMappedTime->gfBias -= 0.01f;
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case 'M':
			if (m_pPostProcessingShader->GetPipelineIndex() == 0)
				m_pPostProcessingShader->SetPipelineIndex(1);
			else
				m_pPostProcessingShader->SetPipelineIndex(0);
			break;
		case 'N':
			if (m_pBlurComputeShader->GetPipeLineIndex() == 0)
				m_pBlurComputeShader->SetPipeLineIndex(1);
			else
				m_pBlurComputeShader->SetPipeLineIndex(0);
			break;
		case VK_UP://		m_pcbMappedLights->bias	0.00119999994	float
			//m_pcbMappedLights->bias += 0.0001f;
			break;
		case VK_DOWN:
			//m_pcbMappedLights->bias -= 0.0001f;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return false;
}

bool CMainScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CMainScene::AnimateObjects(float fElapsedTime, float fCurTime)
{
	m_fElapsedTime = fElapsedTime;
	m_pcbMappedTime->time += fElapsedTime;

	for (auto& shader : m_vShader)
	{
		shader->AnimateObjects(fElapsedTime);
	}

	for (auto& shader : m_vForwardRenderShader)
	{
		shader->AnimateObjects(fElapsedTime);
	}
	
	m_vForwardRenderShader[PARTICLE_SHADER]->ParticleUpdate(fCurTime);

	int light_Id{};
	for (int i = 0;i < MAX_CLIENT;++i) {
		if (m_apPlayer[i]->GetClientId() == -1) {
			continue;
		}
		auto player = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
		if (player) {
			XMFLOAT4X4* xmf4x4playerLight = player->GetFlashLigthWorldTransform();
			m_pLightCamera[i - 1]->m_pLight->m_xmf3Position = XMFLOAT3(xmf4x4playerLight->_41, xmf4x4playerLight->_42, xmf4x4playerLight->_43);//m_pPlayer->GetCamera()->GetPosition();
			m_pLightCamera[i - 1]->m_pLight->m_xmf3Direction = XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23);/*XMFLOAT3(xmf4x4playerLight._31, xmf4x4playerLight._32, xmf4x4playerLight._33);*/ //m_pPlayer->GetCamera()->GetLookVector();

			light_Id++;
		}
	}

	XMFLOAT3 clientCameraPos = m_pMainPlayer->GetCamera()->GetPosition();
	sort(m_pLightCamera.begin() + 4, m_pLightCamera.end(), [clientCameraPos](const shared_ptr<CLightCamera>& A, const shared_ptr<CLightCamera>& B) {
		//const float epsilon = 1e-5f; // ��� ����
		XMFLOAT3 clToA = Vector3::Subtract(clientCameraPos, A->GetPosition());
		XMFLOAT3 clToB = Vector3::Subtract(clientCameraPos, B->GetPosition());
		return Vector3::Length(clToA) < Vector3::Length(clToB);
		});

	for (auto& cm : m_pLightCamera) {
		cm->Update(cm->GetLookAtPosition(), fElapsedTime);
		if (auto player = cm->GetPlayer().lock()) {
			if (player->GetClientId() == -1) {
				cm->m_pLight->m_bEnable = false;
			}
		}
	}

	// �÷��̾ ���� ����
	for (auto& pPlayer : m_apPlayer)
	{
		if (pPlayer->GetClientId() == m_pMainPlayer->GetClientId())
		{
			continue;
		}

		XMFLOAT3 xmf3MainPos = m_pMainPlayer->GetPosition();
		XMFLOAT3 xmf3OtherPos = pPlayer->GetPosition();

		if (abs(xmf3MainPos.y - xmf3OtherPos.y) > 4.0f) // ���� �ٸ��� �ȵ鸲
		{
			pPlayer->SetPlayerVolume(0.0f);
			continue;
		}

		float fWeight = (4.0f - abs(xmf3MainPos.y - xmf3OtherPos.y)) / 4.0f;

		xmf3MainPos.y = 0.0f;
		xmf3OtherPos.y = 0.0f;
		float fDistance = Vector3::Distance(xmf3MainPos, xmf3OtherPos);
		if (fDistance > WALK_SOUND_DISTANCE)
		{
			pPlayer->SetPlayerVolume(0.0f);
		}
		else
		{
			float fVolume = ((WALK_SOUND_DISTANCE - fDistance) / WALK_SOUND_DISTANCE) * fWeight ;
			pPlayer->SetPlayerVolume(fVolume);
		}
	}
}


void CMainScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());
	if (m_pd3dCbvSrvUavDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, m_pd3dCbvSrvUavDescriptorHeap.GetAddressOf());

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);
}

void CMainScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	//PrepareRender(pd3dCommandList, pCamera);

	for (auto& shader : m_vShader)
	{
		shader->Render(pd3dCommandList, pCamera, m_pMainPlayer, nPipelineState);
	}
}

void CMainScene::ShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{

	// �׸��ڸʿ� �ش��ϴ� �ؽ�ó�� ����Ÿ������ ��ȯ
	m_pPostProcessingShader->OnShadowPrepareRenderTarget(pd3dCommandList, m_nLights); //�÷��̾��� ������ 1�� -> [0] ��° ��ҿ� �������.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	//�׸��ڸʿ��� �÷��̾��� �޽��� �׸�.
	for (auto& pl : m_apPlayer) {
		if (pl->GetClientId() == -1) continue;
		pl->SetShadowRender(true);
	}

	int lightId{};
	auto zombie = dynamic_pointer_cast<CZombiePlayer>(m_pMainPlayer);
	if (zombie)
	{
		for (; lightId < MAX_CLIENT - 1; ++lightId)
		{
			//m_pScene->m_pLights[lightId].m_bEnable = false;
			m_pLightCamera[lightId]->m_pLight->m_bEnable = false; // �׻� ��� ���� ������. �׸��ڸ��� �����ϴ� ���� �Ӱ�.
		}
	}
	else {
		for (int i = 0; i < MAX_CLIENT; ++i) {
			if (m_apPlayer[i]->GetClientId() == -1) 
			{
				continue;
			}
			auto survivor = dynamic_pointer_cast<CBlueSuitPlayer>(m_apPlayer[i]);
			if (!survivor) 
			{
				continue;
			}

			//lightId �� ���� �����ϹǷ� �����ڸ� 4������ �����߱⿡ �׻� 0~3 �� �ε����z �������� �� ī�޶�
			XMFLOAT4X4* xmf4x4playerLight = survivor->GetFlashLigthWorldTransform();
			XMFLOAT3 xmf3LightPosition = XMFLOAT3(xmf4x4playerLight->_41, xmf4x4playerLight->_42, xmf4x4playerLight->_43);
			XMFLOAT3 xmf3LightLook = XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23);
			xmf3LightLook = Vector3::ScalarProduct(xmf3LightLook, -1.0f, false);
			xmf3LightPosition = Vector3::Add(xmf3LightPosition, xmf3LightLook);
			m_pLightCamera[i - 1]->SetPosition(xmf3LightPosition);
			m_pLightCamera[i - 1]->SetLookVector(XMFLOAT3(xmf4x4playerLight->_21, xmf4x4playerLight->_22, xmf4x4playerLight->_23));
			m_pLightCamera[i - 1]->RegenerateViewMatrix();
			m_pLightCamera[i - 1]->MultiplyViewProjection();

			static XMFLOAT4X4 xmf4x4ToTexture = {
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
			};
			static XMMATRIX xmProjectionToTexture = XMLoadFloat4x4(&xmf4x4ToTexture);

			XMFLOAT4X4 viewProjection = m_pLightCamera[i - 1]->GetViewProjection();
			XMMATRIX xmmtxViewProjection = XMLoadFloat4x4(&viewProjection);
			//XMStoreFloat4x4(&m_pScene->m_pLights[lightId].m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));
			XMStoreFloat4x4(&m_pLightCamera[i - 1]->m_pLight->m_xmf4x4ViewProjection, XMMatrixTranspose(xmmtxViewProjection * xmProjectionToTexture));
			m_pLightCamera[i - 1]->SetFloor(static_cast<int>(floor(survivor->GetPosition().y / 4.5f)));

			lightId++;
		}
	}

	for (int i = 0; i < m_nLights; ++i)
	{
		m_pLightCamera[i]->m_pLight->m_bEnable = false; // �׻� ��� ���� ������. �׸��ڸ��� �����ϴ� ���� �Ӱ�.
		if (pCamera->GetFloor() != m_pLightCamera[i]->GetFloor()) {
			continue;
		}

		XMFLOAT3 playerToLight = Vector3::Subtract(pCamera->GetPosition(), m_pLightCamera[i]->GetPosition());
		if (Vector3::Length(playerToLight) > 25.0f) { // �Ȱ��� ���� ��ġ�� ���� ���ؼ��� ������ ���� �ʴ´�.
			continue;
		}

		if (pCamera)
		{
			if (!pCamera->IsInFrustum(m_pLightCamera[i]->GetBoundingFrustum()))
			{
				continue;
			}
		}

		m_pLightCamera[i]->m_pLight->m_bEnable = true; // �׻� ��� ���� ������. �׸��ڸ��� �����ϴ� ���� �Ӱ�.

		D3D12_CPU_DESCRIPTOR_HANDLE shadowRTVDescriptorHandle = m_pPostProcessingShader->GetShadowRtvCPUDescriptorHandle(i);
		pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

		pd3dCommandList->OMSetRenderTargets(1, &shadowRTVDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

		//1�� ������
		if (i < MAX_SURVIVOR)
		{
			int pl_id = m_pLightCamera[i]->GetPlayer().lock()->GetClientId();
			if (pl_id == -1) continue;
			m_apPlayer[pl_id]->SetSelfShadowRender(true); //�ڽ��� ����� ���ؼ� �ڽ��� �׸��� ����.
			PrepareRender(pd3dCommandList, m_pLightCamera[i]);
			Render(pd3dCommandList, m_pLightCamera[i], 1); // ī�޶� ���� ��ġ��� �����ؼ� ��������.
			m_apPlayer[pl_id]->SetSelfShadowRender(false);
		}
		else
		{
			ShadowPreRender(pd3dCommandList, m_pLightCamera[i], 1);
		}
	}
	// �׸��ڸʿ� �ش��ϴ� ����Ÿ���� �ؽ�ó�� ��ȯ
	m_pPostProcessingShader->TransitionShadowMapRenderTargetToCommon(pd3dCommandList, m_nLights); //�÷��̾��� ������ 1�� -> [0] ��° ��ҿ� �������.

}

void CMainScene::ShadowPreRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, int nPipelineState)
{
	PrepareRender(pd3dCommandList, pCamera);

	for (auto& shader : m_vPreRenderShader)
	{
		shader->PartitionRender(pd3dCommandList, pCamera, nPipelineState);
	}
	m_vShader[SKINNEDANIMATION_STANDARD_SHADER]->Render(pd3dCommandList, pCamera, m_pMainPlayer, nPipelineState);
}

void CMainScene::FinalRender(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle, int nGameState)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);

	Render(pd3dCommandList, pCamera, 0);

	m_pPostProcessingShader->TransitionRenderTargetToCommon(pd3dCommandList);

	FLOAT ClearValue[4] = { 1.0f,1.0f,1.0f,1.0f };

	ClearValue[3] = 1.0f;
	pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearValue, 0, NULL);

	m_pPostProcessingShader->OnPrepareRenderTargetForLight(pd3dCommandList, 0, NULL, &d3dDsvCPUDescriptorHandle);

	//OM ����Ÿ������ �缳��
	//pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);
	//������ ��ü ���� ������
	pd3dCommandList->SetGraphicsRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);
	m_pPostProcessingShader->Render(pd3dCommandList, pCamera, m_pMainPlayer);

	m_pPostProcessingShader->TransitionRenderTargetToCommonForLight(pd3dCommandList);

	pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);
}

void CMainScene::ForwardRender(int nGameState, ID3D12GraphicsCommandList* pd3dCommandList, const std::shared_ptr<CCamera>& pCamera)
{
	//D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);
	//pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// ���� ��ü ������
	if (nGameState == GAME_STATE::IN_GAME)
	{
		for (auto& shader : m_vForwardRenderShader)
		{
			shader->Render(pd3dCommandList, pCamera, m_pMainPlayer);
		}
	}
	else if (nGameState != GAME_STATE::BLUE_SUIT_WIN || GAME_STATE::ZOMBIE_WIN)
	{
		m_vForwardRenderShader[USER_INTERFACE_SHADER]->Render(pd3dCommandList, pCamera, m_pMainPlayer);
	}
}

void CMainScene::BlurDispatch(ID3D12GraphicsCommandList* pd3dCommandList, const shared_ptr<CCamera>& pCamera, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle)
{
	pd3dCommandList->SetComputeRootSignature(m_pd3dGraphicsRootSignature.Get());
	pd3dCommandList->SetComputeRootDescriptorTable(12, m_d3dTimeCbvGPUDescriptorHandle);
	pCamera->UpdateComputeShaderVariables(pd3dCommandList);

	m_pBlurComputeShader->Dispatch(pd3dCommandList, 0);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pPostProcessingShader->GetDsvCPUDesctriptorHandle(0);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());
	pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	m_pTextureToScreenShaderShader->Render(pd3dCommandList, pCamera, nullptr);
}

void CMainScene::SetParticleTest(float fCurTime) {
	auto particleshader = dynamic_cast<ParticleShader*>(m_vForwardRenderShader[PARTICLE_SHADER].get());
	particleshader->SetParticleTest(fCurTime);
}

void CMainScene::ParticleReadByteTask()
{
	for (auto& ob : sharedobject.m_vParticleObjects) {
		ob->ReadByteTask();
	}
}

void CMainScene::FullScreenProcessingRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	PrepareRender(pd3dCommandList, m_pMainPlayer->GetCamera());

	m_vFullScreenProcessingShader->Render(pd3dCommandList, m_pMainPlayer->GetCamera(), m_pMainPlayer);
}
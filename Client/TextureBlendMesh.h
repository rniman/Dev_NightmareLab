#pragma once
#include "Mesh.h"
//position, uv, normal �� ������ �������ϴ� �޽�

class TextureBlendMesh : public CStandardMesh
{
public:
	TextureBlendMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	~TextureBlendMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
};


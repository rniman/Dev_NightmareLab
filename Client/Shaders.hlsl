struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cAlbedo;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    matrix gmtxInverseViewProjection : packoffset(c8);
    float4 gvCameraPosition : packoffset(c12); // 16����Ʈ�� ���߱� ���� w��: ���� �߰�
    float4 gvFogColor : packoffset(c13);
    float4 gvfFogInfo : packoffset(c14); // START, RANGE, Density, MOD
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8);
};

cbuffer cbFrameInfo : register(b5)
{
    float time : packoffset(c0.x);
    float localTime : packoffset(c0.y);
    float usePattern : packoffset(c0.z);
    
    // [0506] �÷��̾� �ܰ���
    float gfTrackingTime : packoffset(c0.w);
}


#define FRAME_BUFFER_WIDTH 1600
#define FRAME_BUFFER_HEIGHT 1024

SamplerState gssWrap : register(s0);
SamplerState gssWrapPoint : register(s3);

#include "Light.hlsl"

Texture2D AlbedoTexture : register(t0);
Texture2D SpecularTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D MetallicTexture : register(t3);
Texture2D EmissionTexture : register(t4);

Texture2D DFTextureTexture : register(t5);
Texture2D DFNormalTexture : register(t6);
Texture2D<float> DFzDepthTexture : register(t7);
Texture2D DFPositionTexture : register(t8);

Texture2D PatternTexture : register(t11);


#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// ���� ���̴��� �����Ѵ�.
float4 VSMain(uint nVertexID : sv_VertexID) : SV_POSITION
{
    float4 output;

// ������Ƽ��(�ﰢ��)�� �����ϴ� ������ �ε���(SV_VertexID)�� ���� ������ ��ȯ�Ѵ�.
// ������ ��ġ ��ǥ�� ��ȯ�� �� ��ǥ(SV_POSITION)�̴�. ��, ������ǥ���� ��ǥ�̴�.
    if (nVertexID == 0)
    {
        output = float4(0.0, 0.5, 0.5, 1.0);
    }
    else if (nVertexID == 1)
    {
        output = float4(0.5, -0.5, 0.5, 1.0);
    }
    else if (nVertexID == 2)
    {
        output = float4(-0.5, -0.5, 0.5, 1.0);
    }

    return output;
}

// �ȼ� ���̴��� �����Ѵ�.
float4 PSMain(float4 input : SV_POSITION) : SV_TARGET
{
	// ������Ƽ���� ��� �ȼ��� ������ ��������� ��ȯ�Ѵ�.
    return float4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
}

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_INSTANCE_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4x4 ins_transform : INSMATRIX;
};


struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};


VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
    output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

VS_STANDARD_OUTPUT VSInstanceStandard(VS_INSTANCE_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 transformMatrix = mul(input.ins_transform, gmtxGameObject);
    
    output.positionW = mul(float4(input.position, 1.0f), transformMatrix).xyz;
    output.normalW = mul(input.normal, (float3x3) transformMatrix);
    output.tangentW = mul(input.tangent, (float3x3) transformMatrix);
    output.bitangentW = mul(input.bitangent, (float3x3) transformMatrix);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{    
    float4 cTexture : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float zDepth : SV_TARGET2;
    float4 position : SV_Target3;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    
    //float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    //float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cAlbedoColor = gMaterial.m_cAlbedo;
    float4 cSpecularColor = gMaterial.m_cSpecular;
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = gMaterial.m_cEmissive;    
    
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = SpecularTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = NormalTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv);
    
    float4 cColor = (cAlbedoColor * 0.7f) + (cSpecularColor * 0.2f) + (cMetallicColor * 0.05f) + (cEmissionColor * 0.5f);
    
    if (usePattern > 0.0f)
    {
        float2 patternUV;
        patternUV.x = input.uv.x;
        patternUV.y = input.uv.y + frac(time);
    
        float4 patternColor = PatternTexture.Sample(gssWrap, patternUV);
        
        //if (patternColor.x >= 0.1f && patternColor.y >= 0.1f && patternColor.z >= 0.1f)
        {
            cColor *= patternColor;
        }
    }
    
    //float3 vCameraPosition = gvCameraPosition.xyz;
    //float3 vPostionToCamera = vCameraPosition - input.positionW;
    //float fDistanceToCamera = length(vPostionToCamera);
    
    ////float fFogFactor = saturate(((gvfFogInfo.x + gvfFogInfo.y) - fDistanceToCamera) / gvfFogInfo.y);
    //float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2)));
    ////float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, fDistanceToCamera * gvfFogInfo.z));
    //cColor = lerp(gvFogColor, cColor, fFogFactor);
    
    output.cTexture = cColor;
    input.normalW = normalize(input.normalW);
    output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);
    output.zDepth = input.position.z;
    output.position = float4(input.positionW, 1.0f);
    
    return output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b3)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b4)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }
    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
    

    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct PS_POSTPROCESSING_OUT
{
    float4 position : SV_Position;
    float2 uv : UV0;
    float4 uvs[MAX_LIGHTS] : UV1; // ����ȭ�� �ʿ�.. �ȼ����̴����� ������� ���� viewProjection�� ���ؽ����̴����� �ϵ��� �ٲ���.
};

PS_POSTPROCESSING_OUT VSPostProcessing(uint nVertexID : SV_VertexID)
{
    PS_POSTPROCESSING_OUT output;
    
    if (nVertexID == 0)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    if (nVertexID == 1)
    {
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 0.0f);
    }
    if (nVertexID == 2)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f,1.0f);
    }
    if (nVertexID == 3)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    if (nVertexID == 4)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    if (nVertexID == 5)
    {
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 1.0f);
    }
    
    //float4 positionW = mul(output.position, gmtxInverseViewProjection); // ��ũ������ -> �������
    //for (int i = 0; i < MAX_LIGHTS; i++)
    //{
    //    output.uvs[i] = mul(positionW, gLights[i].m_viewProjection);
    //}
    
    return output;
}

float4 PSPostProcessing(PS_POSTPROCESSING_OUT input) : SV_Target
{
    float4 cColor = DFTextureTexture.Sample(gssWrap, input.uv);
    float3 normal = DFNormalTexture.Sample(gssWrap, input.uv);
    float4 position = DFPositionTexture.Sample(gssWrap, input.uv);
    //return cColor;
    float depth =  DFzDepthTexture.Sample(gssWrap, input.uv);
    //return float4(depth, depth, depth, 1.0f);
    //return ShadowMapTexture[0].Sample(gssWrap, input.uv); // ���� ����Ǵ��� Ȯ��
    
    normal = (normal.xyz - 0.5f) / 0.5f; // �븻 ���������� Ȯ���غ��� �ִ� ������ �����Ƿ� �ϴ� �������������� ��ȯ�۾���.
    
    float4 light = Lighting(position, normal);
    
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - position.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    //float fFogFactor = saturate(((gvfFogInfo.x + gvfFogInfo.y) - fDistanceToCamera) / gvfFogInfo.y);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2)));
    //float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, fDistanceToCamera * gvfFogInfo.z));
    cColor = lerp(gvFogColor, cColor * light, fFogFactor);
    
    return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
float4 PSTransparent(VS_STANDARD_OUTPUT input) : SV_Target
{
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
        cSpecularColor = SpecularTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
        cNormalColor = NormalTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
        cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
        cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv);
    
    float4 cColor = (cAlbedoColor * 0.7f) + (cSpecularColor * 0.2f) + (cMetallicColor * 0.05f) + (cEmissionColor * 0.05f);
    
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - input.positionW;
    float fDistanceToCamera = length(vPostionToCamera);
    
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2)));
    cColor = lerp(gvFogColor, cColor, fFogFactor);
    
    float2 uvP = input.position.xy / float2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    float depth = DFzDepthTexture.Sample(gssWrap,uvP);
    clip(depth.x - input.position.z);
    
    
    float4 light = Lighting(input.positionW, input.normalW);
    
    cColor = (light * cColor);
    cColor.w *= 0.5f; // ���İ� ���� 
    return cColor;
}


//[0504] UI Shader
struct VS_USER_INTERFACE_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_USER_INTERFACE_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_USER_INTERFACE_OUTPUT VSUserInterface(VS_USER_INTERFACE_INPUT input)
{
    VS_USER_INTERFACE_OUTPUT output;

    //float4 positionW = float4(input.position, 1.0f); -> ���߿� UI�߰��ɶ� ���� �ϰ� ���ϱ�
    output.position = float4(input.position.xy, 0.0f, 1.0f);
    output.uv = input.uv;
    
    return output;
}

float4 PSUserInterface(VS_USER_INTERFACE_OUTPUT input) : SV_TARGET
{
    float4 cAlbedoColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
    
    cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    cAlbedoColor.b = 1.0f;
    return float4(cAlbedoColor);
}

//[0505] OutLineShader

struct VS_OUT_LINE_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSOutLineMask(VS_OUT_LINE_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for(int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }
    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
    
    //output.positionW -= output.normalW * 0.005f;

    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    
    output.uv = input.uv;

    return (output);
}

VS_STANDARD_OUTPUT VSOutLine(VS_OUT_LINE_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for(int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }
    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
    
    output.positionW += output.normalW * 0.01f * (gfTrackingTime * 0.5f);

    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

    output.uv = input.uv;

    return (output);
}

//float4 PSOutLineMask(VS_STANDARD_OUTPUT input) : SV_TARGET
//{
//    return float4(0.0f, 0.0f, 0.0f, 0.0f);
//}

float4 PSOutLine(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

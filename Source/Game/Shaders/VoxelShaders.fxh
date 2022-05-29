//--------------------------------------------------------------------------------------
// File: VoxelShaders.fxh
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Declare texture array and sampler state array for diffuse texture and normal texture (remove the comment)
--------------------------------------------------------------------*/
Texture2D aTextures[2] : register(t0);
SamplerState aSamplers[2] : register(s0);
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: cbChangesEveryFrame definition (remove the comment)

//--------------------------------------------------------------------------------------*/
cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
};

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
    bool HasNormalMap;
};

cbuffer cbLights : register(b3)
{
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader, 
            instance data included
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: VS_INPUT definition (remove the comment)
--------------------------------------------------------------------*/
struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
};
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
/*--------------------------------------------------------------------
  TODO: PS_INPUT definition (remove the comment)
--------------------------------------------------------------------*/
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
    float3 tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Vertex Shader function VSVoxel definition (remove the comment)
--------------------------------------------------------------------*/
PS_INPUT VSVoxel(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;

    output.Position = mul(input.Position, input.mTransform);
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);
    output.TexCoord = input.TexCoord;
    if (HasNormalMap)
    {
        output.tangent = normalize(mul(float4(input.tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Pixel Shader function PSVoxel definition (remove the comment)
--------------------------------------------------------------------*/
float4 PSVoxel(PS_INPUT input) : SV_Target
{
    float3 normal = normalize(input.Normal);
    
    if (HasNormalMap)
    {
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);

        bumpMap = (bumpMap * 2.0f) - 1.0f;

        float3 bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);

        normal = normalize(bumpNormal);
    }
    
    float3 diffuse = (float3)0;
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        float3 lightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
        diffuse += saturate(max(dot(normal, -lightDirection), 0) * LightColors[i].xyz);
    }
    float4 color = aTextures[0].Sample(aSamplers[0], input.TexCoord);
    return float4((diffuse + ambient) * color, 1.0f);
}
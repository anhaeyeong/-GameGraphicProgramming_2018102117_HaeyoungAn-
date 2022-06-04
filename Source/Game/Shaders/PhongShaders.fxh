//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (1)
#define NEAR_PLANE (0.01f)
#define FAR_PLANE (1000.0f)

Texture2D aTextures[2] : register(t0);
SamplerState aSamplers[2] : register(s0);

Texture2D shadowMapTexture : register(t2);
SamplerState shadowMapSampler : register(s2);

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
	matrix LightViews[NUM_LIGHTS];
	matrix LightProjections[NUM_LIGHTS];
};

struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 tangent : TANGENT;
    float3 Bitangent : BITANGENT;

};

struct PS_PHONG_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float4 LightViewPosition : TEXCOORD1;
};

struct PS_LIGHT_CUBE_INPUT
{
    float4 Position : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSPhong(VS_INPUT input)
{
	/*--------------------------------------------------------------------
	  TODO: Vertex shader code (remove the comment)
	--------------------------------------------------------------------*/
    PS_PHONG_INPUT output = (PS_PHONG_INPUT)0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);
    output.WorldPosition = mul(input.Position, World);
    output.TexCoord = input.TexCoord;
    output.LightViewPosition = mul(input.Position, World);
    output.LightViewPosition = mul(output.LightViewPosition, LightViews[0]);
    output.LightViewPosition = mul(output.LightViewPosition, LightProjections[0]);
    if (HasNormalMap)
    {
        output.Tangent = normalize(mul(float4(input.tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }


    return output;
}

PS_LIGHT_CUBE_INPUT VSLightCube(VS_INPUT input)
{
    PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT)0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    return output;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE))) / FAR_PLANE;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSPhong(PS_PHONG_INPUT input) : SV_Target
{
    /*--------------------------------------------------------------------
      TODO: Pixel shader code (remove the comment)
    --------------------------------------------------------------------*/
    float4 color = aTextures[0].Sample(aSamplers[0], input.TexCoord);
    float3 ambient = float3(0.1f, 0.1f, 0.1f) * color.rgb;
    float2 depthTexCoord;
    depthTexCoord.x = input.LightViewPosition.x / input.LightViewPosition.w / 2.0f + 0.5f;
    depthTexCoord.y = input.LightViewPosition.y / input.LightViewPosition.w / 2.0f + 0.5f;
    float closestDepth = shadowMapTexture.Sample(shadowMapSampler, depthTexCoord).r;
    float currentDepth = input.LightViewPosition.z / input.LightViewPosition.w;

    closestDepth = LinearizeDepth(closestDepth);
    currentDepth = LinearizeDepth(currentDepth);

    if (currentDepth > closestDepth + 0.001f)
        return float4(ambient, 1.0f);

    float3 normal = normalize(input.Normal);

    if (HasNormalMap)
    {
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);

        bumpMap = (bumpMap * 2.0f) - 1.0f;

        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);

        normal = normalize(bumpNormal);
    }
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    float3 specular = (float3)0;
    float3 diffuse = (float3)0;
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        float3 lightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
        float3 reflectDirection = reflect(lightDirection, normal);
        diffuse += saturate(max(dot(normal, -lightDirection), 0) * LightColors[i].xyz);
        specular += saturate(pow(max(dot(-viewDirection, reflectDirection), 0), 20.0f) * LightColors[i]);
    }
    return float4((ambient + diffuse + specular) * color, 1.0f);

}

float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
    return OutputColor;
}
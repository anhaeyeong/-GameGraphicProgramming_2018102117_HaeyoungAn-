//--------------------------------------------------------------------------------------
// File: Shadersasd.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Declare a diffuse texture and a sampler state (remove the comment)
--------------------------------------------------------------------*/
#define NUM_LIGHTS (1)
#define NEAR_PLANE (0.01f)
#define FAR_PLANE (1000.0f)

Texture2D aTexture : register(t0);
SamplerState aSampler : register(s0);

Texture2D shadowMapTexture : register(t2);
SamplerState shadowMapSampler : register(s2);

TextureCube envTexture : register(t3);
SamplerState envSampler : register(s3);


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

struct PointLight
{
    float4 Position;
    float4 Color;
    /*
    matrix View;
    matrix Projection;
    */
    float4 AttenuationDistance;
};

cbuffer cbLights : register(b3)
{
    /*
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
    matrix LightViews[NUM_LIGHTS];
    matrix LightProjections[NUM_LIGHTS];
    */
    PointLight PointLights[NUM_LIGHTS];
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
/*--------------------------------------------------------------------
  TODO: Vertex Shader function VS definition (remove the comment)
--------------------------------------------------------------------*/
PS_PHONG_INPUT VSEnvironmentMap(VS_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT)0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);
    output.WorldPosition = mul(input.Position, World);
    output.TexCoord = input.TexCoord;
    /*output.LightViewPosition = mul(output.LightViewPosition, World);
    output.LightViewPosition = mul(output.LightViewPosition, PointLights[0].View);
    output.LightViewPosition = mul(output.LightViewPosition, PointLights[0].Projection);*/
    output.LightViewPosition = PointLights[0].Position;
    if (HasNormalMap)
    {
        output.Tangent = normalize(mul(float4(input.tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }


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
/*--------------------------------------------------------------------
  TODO: Pixel Shader function PS definition (remove the comment)
--------------------------------------------------------------------*/
float4 PSEnvironmentMap(PS_PHONG_INPUT input) : SV_Target
{
    float4 color = aTexture.Sample(aSampler, input.TexCoord);

    float3 ambient = float3(0.1f, 0.1f, 0.1f) * color.rgb;
    float3 normal = normalize(input.Normal);
    /*
    if (HasNormalMap)
    {
        float4 bumpMap = aTextures[1].Sample(aSamplers[1], input.TexCoord);

        bumpMap = (bumpMap * 2.0f) - 1.0f;

        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);

        normal = normalize(bumpNormal);
    }
    */
    
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    float3 reflectVector = reflect(viewDirection, normal);
    float3 specular = (float3)0;
    float3 diffuse = (float3)0;
    float4 envColor = envTexture.Sample(envSampler, reflectVector);
    float eps = 0.000001f;
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        float3 lightDirection = normalize(input.WorldPosition - PointLights[i].Position.xyz);
        float3 reflectDirection = reflect(lightDirection, normal);
        float r = distance(input.WorldPosition, PointLights[i].Position.xyz);
        float rSquared = r * r;
        rSquared = rSquared + eps;
        float rSquared0 = PointLights[i].AttenuationDistance.z;
        float attenuation = rSquared0 / rSquared;
        //return float4(attenuation, attenuation, attenuation, attenuation);
        float4 lightColor = PointLights[i].Color * attenuation;
        diffuse += max(dot(normal, -lightDirection), 0) * PointLights[i].Color.xyz * lightColor.xyz;
        specular += pow(max(dot(-viewDirection, reflectDirection), 0), 20.0f) * PointLights[i].Color.xyz * lightColor.xyz;
        //diffuse = (diffuse.x / attenuation, diffuse.y / attenuation, diffuse.z / attenuation);
        //specular = (specular.x / attenuation, specular.y / attenuation, specular.z / attenuation);
        //ambient = (ambient.x / attenuation, ambient.y / attenuation, ambient.z / attenuation);
    }
    float envstrength = 0.2f;
    envColor = envColor * envstrength;
    return float4(ambient + diffuse + specular, 1.0f) * color + envColor;
    
}
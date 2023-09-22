#include "ShaderCore.hlsl"


#define MAX_INSTANCES 5
#define MAX_DIRECTIONAL_LIGHTS 3
#define MAX_POINT_LIGHTS 3
#define MAX_SPOT_LIGHTS 3


struct DirectionalLight
{
	float4 Color;
	float4 Direction;
};

struct PointLight
{
	float4 Color;
	float4 Position;
	float Range;
	float3 Attenuation;
};

struct SpotLight
{
	float4 Color;
	float4 Position;
	float4 Direction;
	float Range;
	float3 Attenuation;
	float Cone;
};


cbuffer PixelConstantBuffer : register(b1)
{
	float4 AmbientColor;
	float4 InstanceColors[MAX_INSTANCES];
	DirectionalLight DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight PointLights[MAX_POINT_LIGHTS];
	//SpotLight SpotLights[MAX_SPOT_LIGHTS];
	float Time;
	float3 Padding;
}


Texture2D DiffuxeTexture2D : register(t0);
TextureCube DiffuseTextureCube : register(t1);
SamplerState LinearSampler : register(s0);
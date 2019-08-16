#define MAX_INSTANCES 5
#define MAX_LIGHTS_DIR 3
#define MAX_LIGHTS_PNT 3
#define MAX_LIGHTS_SPT 3

// LIGHT STRUCTS
struct S_LIGHT_DIR
{
    float4 dir;
    float4 color;
};
struct S_LIGHT_PNT
{
    float4 pos;
    float range;
    float3 atten;
    float4 color;
};
struct S_LIGHT_SPT
{
    float4 pos;
    float4 dir;
    float range;
    float cone;
    float3 atten;
    float4 color;
};

// SHADER INPUT
struct S_PSINPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tex : TEXCOORD;
    float4 color : COLOR;
    uint instanceID : SV_INSTANCEID;
    float4 posWrld : WORLDPOSITION;
};

// SHADER VARIABLES
TextureCube txDiffuseCube : register(t0);
SamplerState samplerLinear : register(s0);

// CONSTANT BUFFER
cbuffer ConstantBuffer : register(b1)
{
	float4 ambientColor;
	float4 instanceColors[MAX_INSTANCES];
	S_LIGHT_DIR dLights[MAX_LIGHTS_DIR];
	S_LIGHT_PNT pLights[MAX_LIGHTS_PNT];
	//S_LIGHT_SPT sLights[MAX_LIGHTS_SPT];
	float t;
	float3 pad;
}

// SHADER
float4 main(S_PSINPUT _input) : SV_TARGET
{
    float4 finalColor = txDiffuseCube.Sample(samplerLinear, _input.tex);
    finalColor.a = 1;
    return finalColor;
}
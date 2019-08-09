#define MAX_DLIGHTS 3
#define MAX_PLIGHTS 3
#define MAX_SLIGHTS 3
#define MAX_INSTANCES 5

// LIGHTS
struct S_DLIGHT
{
	float4		dir;
	float4		color;
};

struct S_PLIGHT
{
	float4		pos;
	float4		color;
	float		range;
	float3		atten;
};

struct S_SLIGHT
{
	float4		pos;
	float4		color;
	float3		dir;
	float		range;
	float		cone;
	float3		atten;
};

// SHADER INPUT
struct S_PSINPUT
{
    float4	pos : SV_POSITION;
    float4	color : COLOR;
    float3	norm : NORMAL;
    float3	tex : TEXCOORD;
    uint	instanceID : SV_INSTANCEID;
};

// CONSTANT BUFFER
cbuffer ConstantBuffer : register(b1)
{
	float4		ambientColor;
	float4		instanceColors[MAX_INSTANCES];
	S_DLIGHT	dLights[MAX_DLIGHTS];
	S_PLIGHT	pLights[MAX_PLIGHTS];
	S_SLIGHT	sLights[MAX_SLIGHTS];
	float		t;
	float3		pad;
}

// SHADER
float4 main(S_PSINPUT _input) : SV_TARGET
{
    float4 finalColor = _input.color;
    return finalColor;
}
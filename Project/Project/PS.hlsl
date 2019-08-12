#define MAX_INSTANCES 5
#define MAX_LIGHTS_DIR 3
#define MAX_LIGHTS_PNT 3
#define MAX_LIGHTS_SPT 3

// LIGHT STRUCTS
struct S_LIGHT_DIR
{
	float4	dir;
	float4	color;
};
struct S_LIGHT_PNT
{
	float4	pos;
	float	range;
	float3	atten;
	float4	color;
};
struct S_LIGHT_SPT
{
	float4	pos;
	float4	dir;
	float	range;
	float	cone;
	float3	atten;
	float4	color;
};

// SHADER INPUT
struct S_PSINPUT
{
	float4	pos : SV_POSITION;
	float3	norm : NORMAL;
	float3	tex : TEXCOORD;
	float4	color : COLOR;
	uint	instanceID : SV_INSTANCEID;
};

// CONSTANT BUFFER
cbuffer ConstantBuffer : register(b1)
{
	float4		ambientColor;
	float4		instanceColors[MAX_INSTANCES];
	S_LIGHT_DIR	dLights[MAX_LIGHTS_DIR];
	S_LIGHT_PNT	pLights[MAX_LIGHTS_PNT];
	S_LIGHT_SPT	sLights[MAX_LIGHTS_SPT];
	float		t;

	//float4		dLightDir[3];
	//float4		dLightColor[3];
	//float4		pLightPos[3];
	//float		pLightRange[3];
	//float3		pLightAtten[3];
	//float4		pLightColor[3];
	//float4		sLightPos[3];
	//float4		sLightDir[3];
	//float		sLightRange[3];
	//float		sLightCone[3];
	//float3		sLightAtten[3];
	//float4		sLightColor[3];
}

// SHADER
float4 main(S_PSINPUT _input) : SV_TARGET
{
	float4 finalColor = 0;

	// directional lights
	for (unsigned int i = 0; i < 3; i++)
	{
		finalColor += saturate(dot((float3)dLights[i].dir, _input.norm) * dLights[i].color);
		//finalColor += saturate(dot(float3(1, 0, 0), _input.norm) * float4(1, 0, 0, 1));
	}
	finalColor.a = 1;
	return finalColor;
}
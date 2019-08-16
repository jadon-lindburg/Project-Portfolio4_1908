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
	_input.norm = normalize(_input.norm);
	float4 diffuse = _input.color;
	float4 finalColor = float4(0, 0, 0, 0);
	// point lights
	for (unsigned int j = 0; j < MAX_LIGHTS_PNT; j++)
	{
		float3 lightToPixelVector = pLights[j].pos.xyz - _input.posWrld.xyz;
		float d = length(lightToPixelVector);
		if (d <= pLights[j].range)
		{
			lightToPixelVector /= d;
			float lightIntensity = dot(lightToPixelVector, _input.norm);
			if (lightIntensity > 0)
			{
				finalColor += lightIntensity * diffuse * pLights[j].color;
				finalColor /= pLights[j].atten[0] + (pLights[j].atten[1] * d) + (pLights[j].atten[2] * (d * d));
			}
		}
	}
	// directional lights
	for (unsigned int i = 0; i < MAX_LIGHTS_DIR; i++)
	{
		finalColor += saturate(dot((float3) dLights[i].dir, _input.norm) * dLights[i].color);
	}
	finalColor = saturate(finalColor + (ambientColor * diffuse));
	finalColor.a = 1;
	return finalColor;
}
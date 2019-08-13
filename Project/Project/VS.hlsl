// SHADER INPUT
struct S_VSINPUT
{
    float4	pos : POSITION;
    float3	norm : NORMAL;
    float3	tex : TEXCOORD;
    float4	color : COLOR;
    uint	instanceID : SV_INSTANCEID;
};

// SHADER OUTPUT
struct S_VSOUTPUT
{
    float4	pos : SV_POSITION;
    float3	norm : NORMAL;
    float3	tex : TEXCOORD;
    float4	color : COLOR;
    uint	instanceID : SV_INSTANCEID;
};

// CONSTANT BUFFER
cbuffer ConstantBuffer : register(b0)
{
	matrix	wrld;
	matrix	view;
	matrix	proj;
	matrix	instanceOffsets[5];
	float	t;
	float3	pad;
}

// SHADER
S_VSOUTPUT main(S_VSINPUT _input)
{
	S_VSOUTPUT output = (S_VSOUTPUT) 0;
    output.pos = _input.pos;
    output.norm = _input.norm;
    output.tex = _input.tex;
    output.color = _input.color;
    output.instanceID = _input.instanceID;

	output.pos = mul(instanceOffsets[output.instanceID], output.pos);
	output.pos = mul(output.pos, wrld);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, proj);

    return output;
}
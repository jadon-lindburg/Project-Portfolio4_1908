

struct S_VSInput
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float3 norm : NORMAL;
    float3 tex : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

struct S_VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 norm : NORMAL;
    float3 tex : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

S_VSOutput main(S_VSInput _input)
{
    S_VSOutput output = (S_VSOutput) 0;
    output.pos = _input.pos;
    output.color = _input.color;
    output.norm = _input.norm;
    output.tex = _input.tex;
    output.instanceID = _input.instanceID;



    return output;
}
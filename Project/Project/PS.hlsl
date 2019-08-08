

struct S_PSInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 norm : NORMAL;
    float3 tex : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

float4 main(S_PSInput _input) : SV_TARGET
{
    float4 finalColor = _input.color;
    return finalColor;
}
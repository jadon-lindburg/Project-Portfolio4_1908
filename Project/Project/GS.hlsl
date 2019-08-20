// SHADER INPUT
struct S_GSINPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tex : TEXCOORD;
    float4 color : COLOR;
    uint instanceID : SV_INSTANCEID;
    float4 posWrld : WORLDPOSITION;
};

// SHADER OUTPUT
struct S_GSOUTPUT
{
    float4 pos : SV_POSITION;
    float3 norm : NORMAL;
    float3 tex : TEXCOORD;
    float4 color : COLOR;
    uint instanceID : SV_INSTANCEID;
    float4 posWrld : WORLDPOSITION;
};

// SHADER
[maxvertexcount(3)]
void main(triangle S_GSINPUT _input[3], inout TriangleStream<S_GSOUTPUT> outputStream)
{
    S_GSOUTPUT output = (S_GSOUTPUT) 0;

    float3 offsets[3] =
    {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1)
    };

    for (uint i = 0; i < 3; i++)
    {
        output.pos = _input[i].pos;
        output.norm = _input[i].norm;
        output.tex = _input[i].tex;
        output.color = _input[i].color;
        output.instanceID = _input[i].instanceID;
        output.posWrld = _input[i].posWrld;
        
        outputStream.Append(output);
    }
    
    outputStream.RestartStrip();
}
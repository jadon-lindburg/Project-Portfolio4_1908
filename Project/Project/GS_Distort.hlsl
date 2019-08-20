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
[maxvertexcount(6)]
void main(triangle S_GSINPUT _input[3], inout TriangleStream<S_GSOUTPUT> outputStream)
{
    S_GSOUTPUT output = (S_GSOUTPUT) 0;

    float3 offsets[2] =
    {
        float3(-1, 0, 0),
        float3(1, 0, 0)
    };

    for (uint i = 0; i < 6; i++)
    {
        if (i < 3)
        {
            output.pos = _input[i].pos;
            output.pos = output.pos + float4(offsets[0], 0);
            output.norm = _input[i].norm;
            output.tex = _input[i].tex;
            output.color = _input[i].color;
            output.instanceID = _input[i].instanceID;
            output.posWrld = _input[i].posWrld + float4(offsets[0], 0);
        }
        else
        {
            output.pos = _input[i - 3].pos;
            output.pos = output.pos + float4(offsets[1], 0);
            output.norm = _input[i - 3].norm;
            output.tex = _input[i - 3].tex;
            output.color = _input[i - 3].color;
            output.instanceID = _input[i - 3].instanceID;
            output.posWrld = _input[i - 3].posWrld + float4(offsets[1], 0);
        }
        
        outputStream.Append(output);
    }
    
    outputStream.RestartStrip();
}
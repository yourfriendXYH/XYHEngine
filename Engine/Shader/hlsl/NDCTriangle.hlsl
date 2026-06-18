struct VertexData
{
    float4 postion : POSITIONT;
    float4 texcoord : TEXCOORD0;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct VSOut
{
    float4 position : SV_Position;
    float4 color : TEXCOORD0;
};

cbuffer globalConstants : register(b0)
{
    float4 color;
};

VSOut MainVS(VertexData inVertexData)
{
    VSOut vsOut;
    vsOut.position = inVertexData.postion;
    vsOut.color = inVertexData.texcoord + color;
    
    return vsOut;
}

float4 MainPS(VSOut inPSInput) : SV_Target
{
    return inPSInput.color;
}
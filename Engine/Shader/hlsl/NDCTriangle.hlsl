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
    float4 normal : NORMAL;
};

static const float PI = 3.1415926;

cbuffer globalConstants : register(b0)
{
    float4 color;
};

cbuffer testMatrix1024 : register(b1)
{
    float4x4 modelMatrix;
    float4x4 viewProjMatrix;
    float4x4 normalMatrix;
    float4x4 append[1021];
};

VSOut MainVS(VertexData inVertexData)
{
    VSOut vsOut;
    vsOut.position = mul(viewProjMatrix, mul(modelMatrix, inVertexData.postion));
    vsOut.color = inVertexData.texcoord + color;
    vsOut.normal = mul(normalMatrix, inVertexData.normal);
    return vsOut;
}

float4 MainPS(VSOut inPSInput) : SV_Target
{
    float3 topColor = float3(0.1, 0.4, 0.6);
    float3 bottomColor = float3(0.7, 0.7, 0.7);
    
    float3 n = normalize(inPSInput.normal);
    float theta = asin(n.y);    // -PI/2 - PI/2
    theta /= PI;
    theta += 0.5;
    float3 ambientColor = lerp(bottomColor, topColor, theta);
    float3 diffuseColor = float3(0.0, 0.0, 0.0);
    float3 specularColor = float3(0.0, 0.0, 0.0);
    float3 surfaceColor = ambientColor + diffuseColor + specularColor;
    return float4(surfaceColor, 1.0);
}
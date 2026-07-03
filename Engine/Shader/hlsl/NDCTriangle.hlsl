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
    float4 texcoord : TEXCOORD0;
    float4 normal : NORMAL;
    float4 positionWS : TEXCOORD1;
};

static const float PI = 3.1415926;

cbuffer globalConstants : register(b0)
{
    float4 testValue;
};

cbuffer testMatrix1024 : register(b1)
{
    float4x4 modelMatrix;
    float4x4 viewProjMatrix;
    float4x4 normalMatrix;
    float4x4 append[1021];
};

Texture2D tex : register(t0);

SamplerState samplerState : register(s0);

VSOut MainVS(VertexData inVertexData)
{
    VSOut vsOut;
    vsOut.normal = mul(normalMatrix, inVertexData.normal);
    float3 testPosition = inVertexData.postion.xyz/* + vsOut.normal.xyz * sin(testValue.x)*/;
    vsOut.positionWS = mul(modelMatrix, float4(testPosition, 1.0));
    //vsOut.position = mul(viewProjMatrix, vsOut.positionWS);
    vsOut.position = vsOut.positionWS;
    vsOut.texcoord = inVertexData.texcoord;
    return vsOut;
}

[maxvertexcount(4)]
void MainGS(triangle VSOut input[3], uint inPrimitiveID : SV_PrimitiveID, inout TriangleStream<VSOut> outTriangleStream)
{
    float3 N = normalize(input[0].normal.xyz);
    float3 helperVec = abs(N.y) > 0.999 ? float3(0.0, 0.0, 1.0) : float3(0.0, 1.0, 0.0);
    float3 tangent = normalize(cross(N, helperVec));
    float3 bitangent = normalize(cross(tangent, N));
    
    input[0].position = mul(viewProjMatrix, input[0].positionWS);
    input[1].position = mul(viewProjMatrix, input[1].positionWS);
    input[2].position = mul(viewProjMatrix, input[2].positionWS);
    
    outTriangleStream.Append(input[0]);
    outTriangleStream.Append(input[1]);
    outTriangleStream.Append(input[2]);
}

float4 MainPS(VSOut inPSInput) : SV_Target
{
    float3 bottomColor = float3(0.1, 0.4, 0.6);
    float3 topColor = float3(0.7, 0.7, 0.7);
    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float3 cameraPosition = float3(0.0, 0.0, 2.0);
    
    // 环境光
    float3 n = normalize(inPSInput.normal.xyz);
    float theta = asin(n.y); // -PI/2 - PI/2
    theta /= PI;
    theta += 0.5;
    float ambientColorIntensity = 0.2;
    float3 ambientColor = lerp(bottomColor, topColor, theta) * ambientColorIntensity;
    // 漫反射
    float diffuseIntensity = max(0.0, dot(n, lightDir));
    float3 diffuseLightColor = float3(0.1, 0.4, 0.6);
    float3 diffuseColor = diffuseLightColor * diffuseIntensity;
    // 高光
    float3 specularColor = float3(0.0, 0.0, 0.0);
    if (diffuseIntensity > 0.0) // 有光的地方才有高光
    {
        float3 V = normalize(cameraPosition - inPSInput.positionWS.xyz);
        float3 R = normalize(reflect(-lightDir, n));
        float3 specularIntensity = pow(max(0.0, dot(V, R)), 64.0);
        specularColor = float3(1.0, 1.0, 1.0) * specularIntensity;
    }
    
    float3 surfaceColor = ambientColor + diffuseColor + specularColor/* + tex.Sample(samplerState, inPSInput.texcoord.xy).xyz*/;
    return float4(surfaceColor, 1.0);
}
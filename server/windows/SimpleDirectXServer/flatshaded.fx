float4x4 World;
float4x4 View;
float4x4 Projection;

float4 AmbientColor = float4(1, 1, 1, 1);
float AmbientIntensity = 0.3f;

float3 DiffuseLightDirection = float3(1, 0, 0);
float4 DiffuseColor = float4(1, 1, 1, 1);
float DiffuseIntensity = 0.7f;

texture ModelTexture;
sampler2D textureSampler = sampler_state {
    Texture = (ModelTexture);
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct VertexShaderInput
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 VertexColor : COLOR0;
};

struct VertexShaderOutput
{
    float4 Position : POSITION0;
    float4 PositionWorld : TEXCOORD0;
    float4 VertexColor : COLOR0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 worldPosition = mul(input.Position, World);
    float4 viewPosition = mul(worldPosition, View);
    output.Position = mul(viewPosition, Projection);
    output.PositionWorld = worldPosition;

    output.VertexColor = input.VertexColor;

    return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
    float3 Normal = cross(ddy(input.PositionWorld.xyz), ddx(input.PositionWorld.xyz));
    Normal = normalize(Normal);

    float lightIntensity = dot(Normal, DiffuseLightDirection);
    float4 lightColor = lightIntensity * DiffuseColor * DiffuseIntensity + AmbientColor * AmbientIntensity;
    lightColor.a = 1;

    return saturate(input.VertexColor * lightColor);
}

technique FlatShaded
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 VertexShaderFunction();
        PixelShader = compile ps_3_0 PixelShaderFunction();
    }
}
Texture2D ColorTexture;

SamplerState TrilinearSampler;

cbuffer CBufferPerObject
{
	float4x4 ColorFilter = { 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };
}

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinate : TEXCOORD;
};

float4 main(VS_OUTPUT IN) : SV_Target
{
	float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);

	return float4(mul(color, ColorFilter).rgb, color.a);
}
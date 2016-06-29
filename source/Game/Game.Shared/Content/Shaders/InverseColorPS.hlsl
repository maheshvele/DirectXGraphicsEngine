Texture2D ColorTexture;

SamplerState TrilinearSampler;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinate : TEXCOORD;
};

float4 main(VS_OUTPUT IN) : SV_Target
{
	float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);

	return float4(1 - color.rgb, color.a);
}
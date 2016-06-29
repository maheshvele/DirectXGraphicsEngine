static const float3 GrayScaleIntensity = { 0.299f, 0.587f, 0.114f };

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
	float intensity = dot(color.rgb, GrayScaleIntensity);

	return float4(intensity.rrr, color.a);
}
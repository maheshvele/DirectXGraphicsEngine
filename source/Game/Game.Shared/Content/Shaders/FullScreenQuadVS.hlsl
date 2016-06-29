struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TextureCoordinate : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinate : TEXCOORD;
};

/************* Vertex Shader *************/

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = IN.Position;
	OUT.TextureCoordinate = IN.TextureCoordinate;

	return OUT;
}

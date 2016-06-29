/************* Resources *************/
static const float4 ColorWhite = { 1, 1, 1, 1 };
static const float3 ColorBlack = { 0, 0, 0 };
static const float DepthBias = 0.005;

float3 get_vector_color_contribution(float4 light, float3 color)
{
	// Color (.rgb) * Intensity (.a)
	return light.rgb * light.a * color;
}

float3 get_scalar_color_contribution(float4 light, float color)
{
	// Color (.rgb) * Intensity (.a)
	return light.rgb * light.a * color;
}

cbuffer CBufferPerFrame
{
	float4 AmbientColor;
	float4 LightColor;
	float3 LightPosition;
	float3 CameraPosition;
	float2 ShadowMapSize ;
}
 
cbuffer CBufferPerObject
{
	float4 SpecularColor ;
	float SpecularPower ;
}

Texture2D ColorTexture ;
Texture2D ShadowMap ;

SamplerComparisonState PcfShadowMapSampler;
SamplerState ShadowMapSampler;
SamplerState ColorSampler ;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float2 TextureCoordinate : TEXCOORD0;
	float3 WorldPosition : TEXCOORD1;
	float Attenuation : TEXCOORD2;
	float4 ShadowTextureCoordinate : TEXCOORD3;
};

interface IShadowMappingShader
{
	float4 CalculateFinalColor(VS_OUTPUT IN);
};

class ShadowMappingSimple : IShadowMappingShader
{
	float4 CalculateFinalColor(VS_OUTPUT IN) : SV_Target
	{
		float4 OUT = (float4)0;

		float3 lightDirection = LightPosition - IN.WorldPosition;
		lightDirection = normalize(lightDirection);

		float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

			float3 normal = normalize(IN.Normal);
			float n_dot_l = dot(normal, lightDirection);
		float3 halfVector = normalize(lightDirection + viewDirection);
			float n_dot_h = dot(normal, halfVector);

		float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
			float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

			float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
			float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
			float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;


			if (IN.ShadowTextureCoordinate.w >= 0.0f)
			{
				IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;
				float pixelDepth = IN.ShadowTextureCoordinate.z;
				float sampledDepth = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy).x + DepthBias;

				// Shadow applied in a boolean fashion -- either in shadow or not
				float3 shadow = (pixelDepth > sampledDepth ? ColorBlack : ColorWhite.rgb);
					diffuse *= shadow;
				specular *= shadow;
			}

		OUT.rgb = ambient + diffuse + specular;
		OUT.a = 1.0f;

		return OUT;
	}
};

class ShadowMappingManualWithPCF : IShadowMappingShader
{
	float4 CalculateFinalColor(VS_OUTPUT IN) : SV_Target
	{
		float4 OUT = (float4)0;

		float3 lightDirection = LightPosition - IN.WorldPosition;
		lightDirection = normalize(lightDirection);

		float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

			float3 normal = normalize(IN.Normal);
			float n_dot_l = dot(normal, lightDirection);
		float3 halfVector = normalize(lightDirection + viewDirection);
			float n_dot_h = dot(normal, halfVector);

		float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
			float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

			float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
			float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
			float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;

			if (IN.ShadowTextureCoordinate.w >= 0.0f)
			{
				IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;

				float2 texelSize = 1.0f / ShadowMapSize;
					float sampledDepth1 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy).x + DepthBias;
				float sampledDepth2 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy + float2(texelSize.x, 0)).x + DepthBias;
				float sampledDepth3 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy + float2(0, texelSize.y)).x + DepthBias;
				float sampledDepth4 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy + float2(texelSize.x, texelSize.y)).x + DepthBias;

				float pixelDepth = IN.ShadowTextureCoordinate.z;
				float shadowFactor1 = (pixelDepth > sampledDepth1 ? 0.0f : 1.0f);
				float shadowFactor2 = (pixelDepth > sampledDepth2 ? 0.0f : 1.0f);
				float shadowFactor3 = (pixelDepth > sampledDepth3 ? 0.0f : 1.0f);
				float shadowFactor4 = (pixelDepth > sampledDepth4 ? 0.0f : 1.0f);

				float2 lerpValues = frac(IN.ShadowTextureCoordinate.xy * ShadowMapSize);
					float shadow = lerp(lerp(shadowFactor1, shadowFactor2, lerpValues.x), lerp(shadowFactor3, shadowFactor4, lerpValues.x), lerpValues.y);
				diffuse *= shadow;
				specular *= shadow;
			}

		OUT.rgb = ambient + diffuse + specular;
		OUT.a = 1.0f;

		return OUT;
	}
};

class ShadowMappingWithPCF : IShadowMappingShader
{
	float4 CalculateFinalColor(VS_OUTPUT IN) : SV_Target
	{
		float4 OUT = (float4)0;

		float3 lightDirection = LightPosition - IN.WorldPosition;
		lightDirection = normalize(lightDirection);

		float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

			float3 normal = normalize(IN.Normal);
			float n_dot_l = dot(normal, lightDirection);
		float3 halfVector = normalize(lightDirection + viewDirection);
			float n_dot_h = dot(normal, halfVector);

		float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
			float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

			float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
			float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
			float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;

			IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;
		float pixelDepth = IN.ShadowTextureCoordinate.z;

		float shadow = ShadowMap.SampleCmpLevelZero(PcfShadowMapSampler, IN.ShadowTextureCoordinate.xy, pixelDepth).x;
		diffuse *= shadow;
		specular *= shadow;

		OUT.rgb = ambient + diffuse + specular;
		OUT.a = 1.0f;

		return OUT;
	}
};

IShadowMappingShader ShadowMappingShader;

float4 main(VS_OUTPUT IN) : SV_Target
{
	return ShadowMappingShader.CalculateFinalColor(IN);
}

//float4 main(VS_OUTPUT IN) : SV_Target
//{
//	float4 OUT = (float4)0;
//	
//		float3 lightDirection = LightPosition - IN.WorldPosition;
//		lightDirection = normalize(lightDirection);
//	
//		float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);
//	
//			float3 normal = normalize(IN.Normal);
//			float n_dot_l = dot(normal, lightDirection);
//		float3 halfVector = normalize(lightDirection + viewDirection);
//			float n_dot_h = dot(normal, halfVector);
//	
//		float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
//			float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);
//	
//			float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
//			float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
//			float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;
//	
//	
//			if (IN.ShadowTextureCoordinate.w >= 0.0f)
//			{
//				IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;
//				float pixelDepth = IN.ShadowTextureCoordinate.z;
//				float sampledDepth = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy).x + DepthBias;
//	
//				// Shadow applied in a boolean fashion -- either in shadow or not
//				float3 shadow = (pixelDepth > sampledDepth ? ColorBlack : ColorWhite.rgb);
//					diffuse *= shadow;
//				specular *= shadow;
//			}
//	
//		OUT.rgb = ambient + diffuse + specular;
//		OUT.a = 1.0f;
//	
//		return OUT;
//}
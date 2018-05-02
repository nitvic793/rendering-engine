

struct VertexToPixel
{
	float4 position							: SV_POSITION;
	float2 texturePos						: TEXCOORD0;
	nointerpolation uint selectedTexture	: TEXCOORD1;
	nointerpolation float4 opacity			: TEXCOORD2;

};

Texture2D LensFlareTexture	: register(t1);
SamplerState Sampler		: register(s0);

float4 main(VertexToPixel selection) : SV_TARGET
{
	
	color = texture_2.SampleLevel(sampler_linear_clamp, selection.texPos.xy, 0);
	color *= 1.1 - saturate(selection.texPos.z);
	color *= selection.opacity.x;
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
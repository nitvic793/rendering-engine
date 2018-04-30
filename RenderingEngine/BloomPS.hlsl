/// <summary>
/// Credits: https://github.com/Microsoft/DirectXTK/wiki/Writing-custom-shaders
/// </summary>

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D BaseTexture	: register(t0);
Texture2D BloomTexture	: register(t1);
SamplerState Sampler	: register(s0);

// Helper for modifying the saturation of a color.
float4 AdjustSaturation(float4 color, float saturation)
{
	float grey = dot(color.rgb, float3(0.3, 0.59, 0.11));

	return lerp(grey, color, saturation);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float BloomIntensity = 1.2;
	float BloomSaturation = 1.1;
	float BaseIntensity = 1;
	float BaseSaturation = 1;
	float4 base = BaseTexture.Sample(Sampler, input.uv);
	float4 bloom = BloomTexture.Sample(Sampler, input.uv);

	// Adjust color saturation and intensity.
	bloom = AdjustSaturation(bloom, BloomSaturation) * BloomIntensity;
	base = AdjustSaturation(base, BaseSaturation) * BaseIntensity;

	// Darken down the base image in areas where there is a lot of bloom,
	// to prevent things looking excessively burned-out.
	base *= (1 - saturate(bloom));

	// Combine the two images.
	return base + bloom;
}
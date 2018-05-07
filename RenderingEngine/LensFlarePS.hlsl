

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Pixels			: register(t0);
Texture2D LensFlare			: register(t1);
SamplerState Sampler		: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	
	float4 lensFlare = LensFlare.Sample(Sampler, input.uv);
	return Pixels.Sample(Sampler, input.uv) + lensFlare;
}
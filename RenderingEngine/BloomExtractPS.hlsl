

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float Threshold = 0.9;
	float4 surfaceColor = Pixels.Sample(Sampler, input.uv);
	float4 bright = (surfaceColor - Threshold) / (1 - Threshold);
	surfaceColor = saturate(bright);
	return surfaceColor;
}
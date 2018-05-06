
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Pixels		: register(t0);
Texture2D Radial		: register(t1);
SamplerState Sampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	int ghosts = 3;
	float ghostDispersal = 0.2;
	float haloWidth = 2.9;
	float uDistortion = 0.3;
	float2 invertUV = -input.uv + float2(1, 1);

	float2 ghostVector = (float2(0.5, 0.5) - invertUV) * ghostDispersal;
	float4 result = float4(0, 0, 0, 0);

	for (int i = 0; i < ghosts; ++i)
	{
		float2 offset = frac(invertUV + ghostVector * float(i));
		float2 haloVector = normalize(ghostVector) * haloWidth;
		float weight = length(float2(0.5, 0.5) - offset) / length(float2(0.5, 0.5));
		weight = pow(1 - weight, 5.0);
		result += Pixels.Sample(Sampler, offset) * weight;
	}


	float2 haloVector = normalize(ghostVector) * haloWidth;
	float weight = length(float2(0.5, 0.5) - frac(invertUV + haloVector)) / length(float2(0.5, 0.5));
	weight = pow(1 - weight, 5.0);
	result += Pixels.Sample(Sampler, invertUV + haloVector) * weight;

	result *= Radial.Sample(Sampler, length(float2(0.5, 0.5) - invertUV) / length(float2(0.5, 0.5)));

	return result;
}
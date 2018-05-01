
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

cbuffer externalBlur : register(b0)
{
	float blurValue;
};

Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 totalColor = float4(0,0,0,0);
	uint numSamples = 0;
	float blurAmount = 3.0;
	float pixelWidth = 1.0 / 1280.0;
	float pixelHeight = 1.0 / 720.0;
	for (int x = -blurValue; x <= blurValue; x++)
	{
		for (int y = -blurValue; y <= blurValue; y++)
		{
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);
			totalColor += Pixels.Sample(Sampler, uv);

			numSamples++;
		}
	}

	return totalColor / numSamples;
}
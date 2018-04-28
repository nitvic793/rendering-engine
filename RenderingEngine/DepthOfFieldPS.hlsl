/// <summary>
/// Credits: Mostly converted fx codes from https://digitalerr0r.wordpress.com/2009/05/16/xna-shader-programming-tutorial-20-depth-of-field/
/// </summary>

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

Texture2D Pixels		: register(t0);
Texture2D BlurredPixels	: register(t1);
Texture2D Depth			: register(t2);
SamplerState Sampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float Distance = 2.5;
	float Range = 1.9;
	float Near = 1.2;
	float Far = 0.9;

	/*float Distance = 10;
	float Range = 20;
	float Near = 0.1;
	float Far = 2;*/

	float4 NormalScene = Pixels.Sample(Sampler, input.uv);

	// Get the blurred scene texel
	float4 BlurScene = BlurredPixels.Sample(Sampler, input.uv);

	// Get the depth texel
	float fDepth = BlurredPixels.Sample(Sampler, input.uv).r;

	// Invert the depth texel so the background is white and the nearest objects are black
	fDepth = 1 - fDepth;

	// Calculate the distance from the selected distance and range on our DoF effect, set from the application
	float fSceneZ = (-Near * Far) / (fDepth - Far);
	float blurFactor = saturate(abs(fSceneZ - Distance) / Range);

	// Based on how far the texel is from "distance" in Distance, stored in blurFactor, mix the scene
	return lerp(NormalScene,BlurScene,blurFactor);
}
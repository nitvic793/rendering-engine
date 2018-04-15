
cbuffer externalData : register(b0)
{
	matrix view;
	float3 CameraPosition;
};


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	noperspective float2 screenUV		: TEXCOORD1;
};

// TEXTURE STUFF
Texture2D ScenePixels		: register(t0);
Texture2D NormalMap			: register(t1);
SamplerState BasicSampler	: register(s0);
SamplerState RefractSampler	: register(s1);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Fix for poor normals: re-normalizing interpolated normals
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Sample and unpack normal
	float3 normalFromTexture = NormalMap.Sample(BasicSampler, input.uv).xyz * 2 - 1;

	// Create the TBN matrix which allows us to go from TANGENT space to WORLD space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Overwrite the existing normal (we've been using for lighting),
	// with the version from the normal map, AFTER we convert to world space
	input.normal = normalize(mul(normalFromTexture, TBN));

	// Vars for controlling refraction - Adjust as you see fit
	float indexOfRefr = 0.9f; // Ideally keep this below 1 - not physically accurate, but prevents "total internal reflection"
	float refrAdjust = 0.1f;  // Makes our refraction less extreme, since we're using UV coords not world units
	
	// Calculate the refraction amount in WORLD SPACE
	float3 dirToPixel = normalize(input.worldPos - CameraPosition);
	float3 refrDir = refract(dirToPixel, input.normal, indexOfRefr);

	// Get the refraction XY direction in VIEW SPACE (relative to the camera)
	// We use this as a UV offset when sampling the texture
	float2 refrUV = mul(float4(refrDir, 0.0f), view).xy * refrAdjust;
	refrUV.x *= -1.0f; // Flip the X to point away from the edge (Y already does this due to view space <-> texture space diff)
	
	// Sample the pixels of the render target and return
	return ScenePixels.Sample(RefractSampler, input.screenUV + refrUV);
}
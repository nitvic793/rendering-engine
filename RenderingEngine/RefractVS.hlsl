
// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

// Out of the vertex shader (and eventually input to the PS)
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	noperspective float2 screenUV		: TEXCOORD1;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	// Calculate the world position of this vertex (to be used
	// in the pixel shader when we do point/spot lights)
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;

	// Make sure the normal is in WORLD space, not "local" space
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal); // Make sure it's length is 1

	// Make sure the tangent is in WORLD space and a unit vector
	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	// Pass through the uv
	output.uv = input.uv;

	// Get the screen-space UV
	output.screenUV = (output.position.xy / output.position.w);
	output.screenUV.x = output.screenUV.x * 0.5f + 0.5f;
	output.screenUV.y = -output.screenUV.y * 0.5f + 0.5f;

	return output;
}
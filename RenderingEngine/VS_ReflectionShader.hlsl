cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix reflectionMatrix;
};

struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	//float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};


struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	//float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	//float3 worldPos		: POSITION;
	//float3 tangent		: TANGENT;
	float4 reflectionPosition : TEXCOORD1;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;
	matrix reflectProjectWorld;


	// Change the position vector to be 4 units for proper matrix calculations.
	//input.position.z = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(float4(input.position,1.0), world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, projection);

	// Store the texture coordinates for the pixel shader.
	output.uv = input.uv;

	// Create the reflection projection world matrix.
	reflectProjectWorld = mul(reflectionMatrix, projection);
	reflectProjectWorld = mul(world, reflectProjectWorld);
	
	// Calculate the input position against the reflectProjectWorld matrix.
	output.reflectionPosition = mul(input.position, reflectProjectWorld);

	return output;
}
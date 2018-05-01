
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix shadowView;
	matrix shadowProjection;
};


struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD0;
	float3 tangent		: TANGENT;
	float2 blendUV		: UV;
};


struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD0;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
	float4 shadowPos	: SHADOW;
	float2 blendUV		: UV;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;
	matrix worldViewProj = mul(mul(world, view), projection);

	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	matrix shadowVP = mul(mul(world, shadowView), shadowProjection);
	output.shadowPos = mul(float4(input.position, 1.0f), shadowVP);

	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	output.uv = input.uv;
	output.tangent = normalize(mul(input.tangent, (float3x3)world));
	output.blendUV = input.blendUV;
	return output;
}
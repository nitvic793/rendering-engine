Texture2D heightSRV			: register(t0);
SamplerState heightSampler	: register(s0);



cbuffer externalData : register(b0)
{
	//matrix world;
	matrix view;
	matrix projection;
};

cbuffer externalHeightMapData	: register(b1)
{
	float Hscale;
	float Hbias;
}

// Domian output
struct DomainToPixel
{
	float4 vPosition	: SV_POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float2 uv			: TEXCOORD;
};

// input control point / patch
struct HullToDomain
{
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float2 uv			: TEXCOORD;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DomainToPixel main(
	HS_CONSTANT_DATA_OUTPUT patchTess,//input
	float3 bary : SV_DomainLocation,//domain
	const OutputPatch<HullToDomain, NUM_CONTROL_POINTS> tri)//patch
{
	DomainToPixel dout;

	// Interpolate patch attributes to generated vertices.
	dout.worldPos = bary.x * tri[0].worldPos + bary.y * tri[1].worldPos + bary.z * tri[2].worldPos;
	dout.normal = bary.x * tri[0].normal + bary.y * tri[1].normal + bary.z * tri[2].normal;
	dout.tangent = bary.x * tri[0].tangent + bary.y * tri[1].tangent + bary.z * tri[2].tangent;
	dout.uv = bary.x * tri[0].uv + bary.y * tri[1].uv + bary.z * tri[2].uv;

	// Interpolating normal can unnormalize it, so normalize it.
	dout.normal = normalize(dout.normal);

	//
	// Displacement mapping.
	//

	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	//const float MipInterval = 20.0f;
	//float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

	// Sample height map (stored in alpha channel).
	float h = heightSRV.SampleLevel(heightSampler, dout.uv, 0).a;
	float gHeightScale = 3;
	// Offset vertex along normal.
	dout.worldPos += (gHeightScale * (h - 1.0)) * dout.normal;

	// Project to homogeneous clip space.
	matrix viewProj = mul(view, projection);
	dout.vPosition = mul(float4(dout.worldPos, 1.0f), viewProj);

	return dout;
}

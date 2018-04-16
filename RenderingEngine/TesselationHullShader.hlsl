cbuffer externalData : register(b0)
{
	float tessellationAmount;
}
// Input control point
struct VertexToHull
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
	float tessFactor	: TESS;

};

// Output control point
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

// Patch Constant Function
// For the triangular domain, there are 4 factors (3 sides, 1 inner).
// For the quadrilateral domain, there are 6 factors (4 sides, 2 inner).
// For the isoline domain, there are 2 factors (detail and density).
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VertexToHull, NUM_CONTROL_POINTS> patch,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT pt;

	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].tessFactor + patch[2].tessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].tessFactor + patch[0].tessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].tessFactor + patch[1].tessFactor);
	pt.InsideTess = pt.EdgeTess[0];

	return pt;
}

[domain("tri")]								// set triangle domain
[partitioning("fractional_odd")]			// intrepret tesselation factor options are - int,fractional_even, fractional_odd and pow2
[outputtopology("triangle_cw")]				// tells the tessellator what kind of primitives we want to deal with after tessellation options - triangle ccw, line
[outputcontrolpoints(3)]					// how many control points we will be outputting from the hull program.
[patchconstantfunc("CalcHSPatchConstants")] // this attribute specifies the name of the patch constant function, which is executed once per patch.
HullToDomain main(
	InputPatch<VertexToHull, NUM_CONTROL_POINTS> p,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HullToDomain Output;

	// Insert code to compute Output here
	Output.normal = p[i].normal;
	Output.tangent = p[i].tangent;
	Output.worldPos = p[i].worldPos;
	Output.uv = p[i].uv;

	return Output;
}
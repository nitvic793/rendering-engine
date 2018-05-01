cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct Bones
{
	matrix BoneTransform;
	matrix InvBoneTransform;
};


cbuffer bones : register(b1)
{
	Bones bones[20];
	//Bones bones2[20];
	//int instanceNumber;
}


struct VertexShaderInput
{

	float4 position		: POSITION;     
	float3 normal       : NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
	float4 boneid		: BONEID;
	float4 weight		: WEIGHT;
};


struct VertexToPixel
{

	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
};



VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix worldViewProj = mul(mul(world, view), projection);

	matrix bonetransform = 0;

	
	//if(instanceNumber == 1)
	{
		if (input.boneid.x != -1)
		{
			bonetransform = mul(mul(bones[input.boneid.x].BoneTransform, input.weight.x), bones[input.boneid.x].InvBoneTransform);
		}
		if (input.boneid.y != -1)
		{
			bonetransform += mul(mul(bones[input.boneid.y].BoneTransform, input.weight.y), bones[input.boneid.y].InvBoneTransform);
		}
		if (input.boneid.z != -1)
		{
			bonetransform += mul(mul(bones[input.boneid.z].BoneTransform, input.weight.z), bones[input.boneid.z].InvBoneTransform);
		}
		if (input.boneid.w != -1)
		{
			bonetransform += mul(mul(bones[input.boneid.w].BoneTransform, input.weight.w), bones[input.boneid.w].InvBoneTransform);
		}
	}
	/*
	else 
	{
		if (input.boneid.x != -1)
		{
			bonetransform = mul(mul(bones2[input.boneid.x].BoneTransform, input.weight.x), bones2[input.boneid.x].InvBoneTransform);
		}
		if (input.boneid.y != -1)
		{
			bonetransform += mul(mul(bones2[input.boneid.y].BoneTransform, input.weight.y), bones2[input.boneid.y].InvBoneTransform);
		}
		if (input.boneid.z != -1)
		{
			bonetransform += mul(mul(bones2[input.boneid.z].BoneTransform, input.weight.z), bones2[input.boneid.z].InvBoneTransform);
		}
		if (input.boneid.w != -1)
		{
			bonetransform += mul(mul(bones2[input.boneid.w].BoneTransform, input.weight.w), bones2[input.boneid.w].InvBoneTransform);
		}
	}
	*/

	output.position = mul(mul(bonetransform, input.position), worldViewProj);

	output.normal = mul(mul(bonetransform, input.normal), (float3x3)world);

	output.worldPos = (mul(float4(input.position.x, input.position.y, input.position.z, 1.0f), world)).xyz;

	output.uv = input.uv;

	output.tangent = normalize(mul(mul(bonetransform, input.tangent), (float3x3)world));


	return output;
}
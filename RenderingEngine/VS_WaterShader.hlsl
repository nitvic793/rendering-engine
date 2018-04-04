cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float time;
};


struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
};


struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;
	matrix worldViewProj = mul(mul(world, view), projection);

	//----------------------------Water Motion-----------------------------------
	int u = 64;
	int v = 64;

	float s_X = 0;
	float s_Y = 0;
	float s_Z = 0;

	output.uv = float2(input.position[0] * 22.0 / 2816.0, input.position[2] * 16.5 / 2112.0);


	float2 X0 = float2(input.position[0], input.position[2]);
	float height = 0;
	for (int i = 0; i < 3; i++)
	{
		float2 K = float2(1, i);				// K is wave vector
		float w = 1.5 * (i + 1) / 3.0;			// w is frequency
		float a = 0.01;							// a is amplitude
		float2 X = X0 - K * a*sin(dot(K, X0) - w * time * 10);
		float  y = a * cos(dot(K, X0) - w * time * 10);
		s_X += X[0] / 64 - 0.5f;
		s_Y += X[1] / 64 - 0.5f;
		s_Z += y;
		
	}
	input.position[0] = s_X;
	input.position[2] = s_Y;
	input.position[1] = s_Z;
	input.position *= 7.5f;
	input.normal = float4(0,1,0,1);
	//input.normal = normalize(cross(ddx_coarse(input.position.xyz), ddy_coarse(input.position.xyz));
	//---------------------------------------------------------------
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	output.uv = input.uv;
	output.tangent = normalize(mul(input.tangent, (float3x3)world));
	return output;
}
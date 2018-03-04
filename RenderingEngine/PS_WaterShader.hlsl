struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float4 color		: COLOR;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(0,0,1,1);
}
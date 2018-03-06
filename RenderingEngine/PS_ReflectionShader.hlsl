
//Texture2D shaderTexture;
Texture2D reflectionTexture;
SamplerState SampleType;

struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
										//float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	//float3 worldPos		: POSITION;
	//float3 tangent		: TANGENT;
	float4 reflectionPosition : TEXCOORD1;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float2 reflectTexCoord;
	float4 reflectionColor;
	float4 color;


	// Sample the texture pixel at this location.
	//float4 textureColor = shaderTexture.Sample(SampleType, input.uv);

	// Calculate the projected reflection texture coordinates.
	reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
	reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
	// Sample the texture pixel from the reflection texture using the projected texture coordinates.
	reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);


	return reflectionColor;
}
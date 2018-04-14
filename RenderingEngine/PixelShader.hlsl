
#define MAX_LIGHTS 32

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
	float Padding;
};

struct PointLight
{
	float4 Color;
	float3 Position;
	float Range;
};

cbuffer externalData : register(b0)
{
	DirectionalLight dirLights[MAX_LIGHTS];
	PointLight pointLights[MAX_LIGHTS];
	int DirectionalLightCount;
	int PointLightCount;
	float3 cameraPosition;
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D roughnessTexture : register(t2);
//Texture2D shadowMapTexture	: register(t3);
SamplerState basicSampler : register(s0);
//SamplerComparisonState shadowSampler : register(s1);


// Range-based attenuation function
float Attenuate(float3 lightPosition, float lightRange, float3 worldPos)
{
	float dist = distance(lightPosition, worldPos);

	// Ranged-based attenuation
	float att = saturate(1.0f - (dist * dist / (lightRange * lightRange)));

	// Soft falloff
	return att * att;
}

float calculateSpecular(float3 normal, float3 worldPos, float3 dirToLight, float3 camPos)
{
	float3 dirToCamera = normalize(camPos - worldPos);
	float3 halfwayVector = normalize(dirToLight + dirToCamera);
	//float3 refl = reflect(-dirToLight, normal);
	float shininess = 64;
	//float spec = pow(saturate(dot(dirToCamera, refl)), shininess);
	return shininess == 0 ? 0.0f : pow(max(dot(halfwayVector, normal), 0), shininess);
	//return spec;
}

float4 calculateDirectionalLight(float3 normal, float3 worldPos, DirectionalLight light, float roughness)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	float spec = calculateSpecular(normal, worldPos, dirToLight, cameraPosition) * roughness;
	return spec + light.DiffuseColor * NdotL + light.AmbientColor;
}

float4 calculatePointLight(float3 normal, float3 worldPos, PointLight light, float roughness)
{
	float3 dirToPointLight = normalize(light.Position - worldPos);
	float pointNdotL = dot(normal, dirToPointLight);
	pointNdotL = saturate(pointNdotL);
	float spec = calculateSpecular(normal, worldPos, dirToPointLight, cameraPosition) * roughness;
	return spec + light.Color * pointNdotL;
}

float3 calculateNormalFromMap(float2 uv, float3 normal, float3 tangent)
{
	float3 normalFromTexture = normalTexture.Sample(basicSampler, uv).xyz;
	float3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - N * dot(tangent, N));
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	return normalize(mul(unpackedNormal, TBN));
}


float4 main(VertexToPixel input) : SV_TARGET
{
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);
	float3 finalNormal = calculateNormalFromMap(input.uv, input.normal, input.tangent);
	input.normal = normalize(input.normal);
	finalNormal = normalize(finalNormal);
	float4 totalColor = float4(0, 0, 0, 0);
	float roughness = roughnessTexture.Sample(basicSampler, input.uv).r;

	int i = 0;
	for (i = 0; i < DirectionalLightCount; ++i)
	{
		totalColor += calculateDirectionalLight(finalNormal, input.worldPos, dirLights[i], roughness) * surfaceColor;
	}

	for (i = 0; i < PointLightCount; ++i)
	{
		totalColor += calculatePointLight(finalNormal, input.worldPos, pointLights[i], roughness)  * surfaceColor;
	}
	return totalColor;
}
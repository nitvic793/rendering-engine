
#define MAX_RIPPLES 32
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

struct RippleData {
	float3 ripplePosition;
	float rippleRadius;
	float ringSize;
	float rippleIntensity;
	float padding1;
	float padding2;
};

cbuffer externalData : register(b0)
{
	DirectionalLight dirLights[MAX_LIGHTS];
	PointLight pointLights[MAX_LIGHTS];	
	int DirectionalLightCount;
	int PointLightCount;
	float3 cameraPosition;
	float translate;
	RippleData ripples[MAX_RIPPLES];
	int rippleCount;
	float transparency;
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState basicSampler : register(s0);
Texture2D roughnessTexture : register(t2);
TextureCube SkyTexture		: register(t3);
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
	float spec = calculateSpecular(normal, worldPos, dirToLight, cameraPosition);
	return spec + light.DiffuseColor * NdotL + light.AmbientColor;
}

float4 calculatePointLight(float3 normal, float3 worldPos, PointLight light, float roughness)
{
	float3 dirToPointLight = normalize(light.Position - worldPos);
	float pointNdotL = dot(normal, dirToPointLight);
	pointNdotL = saturate(pointNdotL);
	float spec = calculateSpecular(normal, worldPos, dirToPointLight, cameraPosition);
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

float4 calculateSkyboxReflection(float3 normal, float3 worldPos, float3 dirToLight)
{
	float3 refl = reflect(-dirToLight, normal);
	float3 dirToCamera = normalize(cameraPosition - worldPos);
	float pointLightSpecular = pow(saturate(dot(refl, dirToCamera)), 64);

	return SkyTexture.Sample(basicSampler,
		reflect(-dirToCamera, normal));
}

float calculateDistance(float3 pos1, float3 pos2) {
	float3 dist = pos2 - pos1;
	return sqrt((pow(dist.x, 2) + pow(dist.z, 2)));
	//return sqrt(dist.x ^ 2 + dist.y ^ 2);
}

float4 main(VertexToPixel input) : SV_TARGET
{

	input.uv.x += translate;
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);
	float3 finalNormal = calculateNormalFromMap(input.uv, input.normal, input.tangent);
	input.normal = normalize(input.normal);
	finalNormal = normalize(finalNormal);
	float4 totalColor = float4(0, 0, 0, 0);
	float roughness = roughnessTexture.Sample(basicSampler, input.uv).r;
	//return (calculateDirectionalLight(finalNormal, input.worldPos, dirLights[0], roughness) + calculateSkyboxReflection(input.normal, input.worldPos, dirLights[0].Direction)) *surfaceColor;
	//return calculateSkyboxReflection(input.normal, input.worldPos, dirLights[0].Direction); 

	for (int r = 0; r < rippleCount; ++r) {

		float3 ripplePosition = ripples[r].ripplePosition;
		float rippleRadius = ripples[r].rippleRadius;
		float ringSize = ripples[r].ringSize;
		float rippleIntensity = ripples[r].rippleIntensity;

		//Handle ripples
		//y position shouldn't matter for now
		float3 pos1 = input.worldPos;
		float3 pos2 = ripplePosition;
		pos1.y = 0.0f;
		pos2.y = 0.0f;
		float distance = calculateDistance(pos1, pos2);

		if (distance >= rippleRadius - 0.5f * ringSize && distance <= rippleRadius + 0.5f * ringSize) {
			//Set t to be from 0 to 1
			float t_01 = (distance - (rippleRadius - 0.5f * ringSize)) / ringSize;
			//Set t to be from -1 to 1
			float t = (t_01 - 0.5f) * 0.5f;
			//ripple position to pixel position
			float3 direction = normalize(pos1 - pos2);

			float y = sin(t_01 * 3.14159);

			float3 rippleNormal = normalize(float3(direction.x * t, 0.5f + 0.5f * y, direction.z * t));

			// 0 - finalNormal, 1 - rippleNormal
			finalNormal.x = lerp(finalNormal.x, rippleNormal.x, rippleIntensity);
			finalNormal.y = lerp(finalNormal.y, rippleNormal.y, rippleIntensity);
			finalNormal.z = lerp(finalNormal.z, rippleNormal.z, rippleIntensity);

			finalNormal = normalize(finalNormal);
		}
	}

	int i = 0;
	for (i = 0; i < DirectionalLightCount; ++i)
	{
	
		totalColor += (calculateDirectionalLight(finalNormal, input.worldPos, dirLights[i], roughness)  * calculateSkyboxReflection(finalNormal, input.worldPos, dirLights[i].Direction)) * surfaceColor;
	}

	for (i = 0; i < PointLightCount; ++i)
	{
		float3 dirToLight = normalize(pointLights[i].Position - input.worldPos);
		totalColor += (calculatePointLight(finalNormal, input.worldPos, pointLights[i], roughness) * calculateSkyboxReflection(finalNormal, input.worldPos, dirToLight))  *surfaceColor;
	}

	return float4(totalColor.xyz, transparency);
}

#define MAX_RIPPLES 32
#define MAX_LIGHTS 32

cbuffer waterPixelBuffer : register(b1)
{
	matrix view;
	float3 CameraPosition;
};

struct DomainToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
	noperspective float2 screenUV		: TEXCOORD1;
	//float tessFactor : TESS;
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

Texture2D diffuseTexture	: register(t0);
Texture2D normalTexture		: register(t1);
Texture2D roughnessTexture	: register(t2);
TextureCube SkyTexture		: register(t3);
Texture2D normalTextureTwo	: register(t4);
Texture2D ScenePixels		: register(t5);
Texture2D waterSplash		: register(t6);
SamplerState basicSampler	: register(s0);
SamplerState RefractSampler	: register(s1);
//Texture2D shadowMapTexture	: register(t3);
//SamplerComparisonState shadowSampler : register(s1);

// -----------------------------------------------------
// Range-based attenuation function
// -----------------------------------------------------
float Attenuate(float3 lightPosition, float lightRange, float3 worldPos)
{
	float dist = distance(lightPosition, worldPos);

	// Ranged-based attenuation
	float att = saturate(1.0f - (dist * dist / (lightRange * lightRange)));

	// Soft falloff
	return att * att;
}

// -----------------------------------------------------
// Specular light calculation
// -----------------------------------------------------
float calculateSpecular(float3 normal, float3 worldPos, float3 dirToLight, float3 camPos)
{
	float shininess = 64;
	float3 dirToCamera = normalize(camPos - worldPos);
	float3 refl = reflect(-dirToLight, normal);
	float spec = pow(saturate(dot(dirToCamera, refl)), shininess);
	return spec;
}

// -----------------------------------------------------
// Directional light calculation
// -----------------------------------------------------
float4 calculateDirectionalLight(float3 normal, float3 worldPos, DirectionalLight light)
{
	float3 dirToLight = normalize(-light.Direction);
	float NdotL = dot(normal, dirToLight);
	NdotL = saturate(NdotL);
	float spec = calculateSpecular(normal, worldPos, dirToLight, cameraPosition);
	return saturate( spec + light.DiffuseColor * NdotL + light.AmbientColor);
}

// -----------------------------------------------------
// Point light calculation
// -----------------------------------------------------
float4 calculatePointLight(float3 normal, float3 worldPos, PointLight light)
{
	float3 dirToPointLight = normalize(light.Position - worldPos);
	float pointNdotL = dot(normal, dirToPointLight);
	pointNdotL = saturate(pointNdotL);
	float spec = calculateSpecular(normal, worldPos, dirToPointLight, cameraPosition);
	return saturate(spec + light.Color * pointNdotL);
}

// ----------------------------------------------------------
// Calculate the two normals separately and add then together
// I was too lazy to abstract it out
// ----------------------------------------------------------
float3 calculateNormalFromMap(float2 uv, float3 normal, float3 tangent)
{
	float3 normalFinal;
	uv.x += translate ;
	float3 normalFromTexture = normalTexture.Sample(basicSampler, uv).xyz;
	float3 unpackedNormal = normalFromTexture * 2.0f - 1.0f;
	float3 N = normal;
	float3 T = normalize(tangent - N * dot(tangent, N));
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	normalFinal = normalize(mul(unpackedNormal, TBN));

	uv.y += translate;
	float3 normalFromTexture2 = normalTextureTwo.Sample(basicSampler, uv).xyz;
	unpackedNormal = normalFromTexture2 * 2.0f - 1.0f;
	N = normal;
	T = normalize(tangent - N * dot(tangent, N));
	B = cross(N, T);
	TBN = float3x3(T, B, N);
	normalFinal = normalize(normalFinal + normalize(mul(unpackedNormal, TBN)));
	return normalFinal;
}

// -----------------------------------------------------
// Reflect the skybox
// -----------------------------------------------------
float4 calculateSkyboxReflection(float3 normal, float3 worldPos)
{
	float3 dirToCamera = normalize(cameraPosition - worldPos);
	return SkyTexture.Sample(basicSampler,reflect(-dirToCamera, normal));
}

// -----------------------------------------------------
// Return distance between two vector3s
// -----------------------------------------------------
float calculateDistance(float3 pos1, float3 pos2) {
	float3 dist = pos2 - pos1;
	return sqrt((pow(dist.x, 2) + pow(dist.z, 2)));
	//return sqrt(dist.x ^ 2 + dist.y ^ 2);
}

// -----------------------------------------------------
// Calculate the refract UV
// -----------------------------------------------------
float2 calculateRefraction(float3 normal, float3 worldPos)
{
	// Vars for controlling refraction - Adjust as you see fit
	float indexOfRefr = 0.9f; // Ideally keep this below 1 - not physically accurate, but prevents "total internal reflection"
	float refrAdjust = 1.0f;  // Makes our refraction less extreme, since we're using UV coords not world units

							  // Calculate the refraction amount in WORLD SPACE
	float3 dirToPixel = normalize(worldPos - CameraPosition);
	float3 refrDir = refract(dirToPixel, normal, indexOfRefr);
	
	// Get the refraction XY direction in VIEW SPACE (relative to the camera)
	// We use this as a UV offset when sampling the texture
	float2 refrUV = mul(float4(refrDir, 0.0f), view).xy * refrAdjust;
	refrUV.x *= -1.0f; // Flip the X to point away from the edge (Y already does this due to view space <-> texture space diff)
	return refrUV;
}

// Schlick's approximation
// https://en.wikipedia.org/wiki/Schlick's_approximation
float Fresnel(float3 normal, float3 worldPos)
{
	float3 dirToPixel = normalize(cameraPosition - worldPos);
	float cosTheta = dot(normal, -dirToPixel);
	float n1 = 1.0f;	// index of refraction of air
	float n2 = 0.9f;	// index of refraction of water
	float R0 = pow(((n1 - n2) / (n1 + n2)),2) ;

	float R = R0 + (1 - R0)* pow((1 - saturate(cosTheta)), 5);
	return cosTheta;
}

float3 Vector3Lerp(float3 a, float3 b, float i) {
	float3 newVector = float3(0, 0, 0);
	newVector.x = lerp(a.x, b.x, i);
	newVector.y = lerp(a.y, b.y, i);
	newVector.z = lerp(a.z, b.z, i);
	return newVector;
}

float3 CalculateRipple(float3 pixelPos, float3 ripplePos, float rippleRadius, float3 normal) {
	float3 finalNormal = normal;
	float ringSize = 5;
	float frequency = 30;
	float outerRingRadius = rippleRadius;
	float innerRingRadius = rippleRadius - ringSize;
	float distance = calculateDistance(pixelPos, ripplePos);
	float intensity = (distance - innerRingRadius) / (outerRingRadius - innerRingRadius);
	intensity = max(intensity, 0);
	if (distance <= outerRingRadius && distance >= innerRingRadius) {
		float3 outward = normalize(float3(pixelPos.x - ripplePos.x, 1, pixelPos.z - ripplePos.z));
		float3 inward = normalize(float3(ripplePos.x - pixelPos.x, 1, ripplePos.z - pixelPos.z));
		
		float i = (cos((1 - intensity) * frequency) * 0.5) + 0.5;
		finalNormal = Vector3Lerp(outward, inward, i);
	}
	finalNormal = Vector3Lerp(normal, finalNormal, intensity);
	finalNormal = normalize(finalNormal);
	return finalNormal;
}

float4 main(DomainToPixel input) : SV_TARGET
{
	// Fix normals and tangents
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);
	
	// Sample, blend and translate multiple normals
	float3 finalNormal = calculateNormalFromMap(input.uv, input.normal, input.tangent);
	finalNormal = normalize(finalNormal);
	//return float4(finalNormal, 1);
	// Calculate refraction
	float2 refractUV = calculateRefraction(finalNormal,input.worldPos);

	float4 totalColor = float4(0, 0, 0, 0);
	float roughness = roughnessTexture.Sample(basicSampler, input.uv).r;
	
	for (int r = 0; r < rippleCount; ++r) {

		float3 ripplePosition = ripples[r].ripplePosition;
		float rippleRadius = ripples[r].rippleRadius;

		//Handle ripples
		//y position shouldn't matter for now
		float3 pos1 = input.worldPos;
		float3 pos2 = ripplePosition;
		pos1.y = 0.0f;
		pos2.y = 0.0f;
		float distance = calculateDistance(pos1, pos2);

		finalNormal = normalize(finalNormal);
		float4 foam = float4(1, 0, 0, 1);
		if (ripplePosition.x == input.position.x && ripplePosition.z == input.position.z)
			return foam;
		
		finalNormal = CalculateRipple(input.worldPos, ripplePosition, rippleRadius, finalNormal);
	}

	float reflectionCoeff = normalize(Fresnel(finalNormal, input.worldPos));
	float4 reflection = float4(0, 0, 0, 0);

	// Lighting calculation
	int i = 0;
	for (i = 0; i < DirectionalLightCount; ++i)
	{
		totalColor += calculateDirectionalLight(finalNormal, input.worldPos, dirLights[i]);
	}

	for (i = 0; i < PointLightCount; ++i)
	{
		totalColor += calculatePointLight(finalNormal, input.worldPos, pointLights[i]);
	}
	reflection = calculateSkyboxReflection(finalNormal, input.worldPos);
	
	//if (rippleCount > 0)
	//{
	//	float dist = calculateDistance(ripples[0].ripplePosition, input.worldPos);
	//	if (dist < 2) 
	//	{
	//		float4 a = waterSplash.Sample(basicSampler,input.uv);
	//		//if (a.r > 0.7 || a.g > 0.7 || a.b > 0.7)
	//			return lerp(totalColor,float4(1,1,1,1),1);
	//	}
	//}
	
	// Gamma correction
	surfaceColor.rgb = lerp(surfaceColor.rgb, pow(surfaceColor.rgb, 2.2), 1);
	//float3 gammaCorrectValue = lerp(totalColor, pow(totalColor, 1.0f / 2.2f), 1);
	//return (float4(saturate(gammaCorrectValue), 1) + (reflection * (1 - reflectionCoeff) + reflectionCoeff * ScenePixels.Sample(RefractSampler, input.screenUV + refractUV * 0.1f) ) ) * surfaceColor * 0.4f;
	float3 gammaCorrectValue = (float3)lerp(totalColor, pow(totalColor, 1.0f / 2.2f), 1);
	return float4(gammaCorrectValue, 1) * (reflection * ScenePixels.Sample(RefractSampler, input.screenUV + refractUV * 0.1f)) * (reflectionCoeff + (1 - reflectionCoeff)) + surfaceColor * 0.2;
}
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float time;
};

// gMaxTessDistance < gMinTessDistance as we increase the tessellation when the distance from the eye decreases
cbuffer cbPerFrame : register(b1)
{
	float gMaxTessDistance;	//	distance from the eye where max tesselation is achieved
	float gMinTessDistance;	//	distance from the eye where min tessalation is achieved
	float gMinTessFactor;	//	minimum amount of tesselation
	float gMaxTessFactor;	//	maximum amount of tesselation
};

struct VertexShaderInput
{
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;
};


struct VertexToHull
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
	float tessFactor	: TESS;
};
struct Wave
{
	float2 direction;
	float amplitude;
	float wavelength;
};
cbuffer WaveInfo	:	register(b2)
{
	Wave waves[100];
};

static int numWaves = 10;
static float steepness = 0;
static float speed = 15;
float3 CalculateGerstnerWave(float3 inputVertex)
{
	float3 total = float3(0, 0, 0);

	for (int i = 0; i < numWaves; i++)
	{
		Wave wave = waves[i];
		// Wavelength (L): the crest-to-crest distance between waves in world space. Wavelength L relates to frequency w as w = 2/L.
		float wi = 2 * 3.1416 / wave.wavelength;
		// Amplitude (A): the height from the water plane to the wave crest.
		float ai = wave.amplitude;
		// Speed (S): the distance the crest moves forward per second.
		// It is convenient to express speed as phase-constant, where phase-constant = S x 2/L
		float phi = speed * wi;
		// direction normalized
		float2 direction = normalize(wave.direction);
		// Angle calculated
		// Water mesh is in the xz plane not xy plane as the gerstner equation suggests
		float theta = wi * dot( inputVertex.xz, direction) + phi * time;
		// Qi is a parameter that controls the steepness of the waves.
		//	For a single wave i, Qi of 0 gives the usual rolling sine wave, and Qi = 1/(wi Ai ) gives a sharp crest.
		//  range of 0 to 1, and using Qi = Q/(wi Ai x numWaves) to vary from totally smooth waves to the sharpest waves
		float qi = steepness / wi * wave.amplitude * numWaves;
		
		// X and Z values as per gerstner equation
		total.x += inputVertex.x + qi * ai * direction.x * cos(theta);
		total.y += wave.amplitude * sin(theta);
		total.z += inputVertex.z + qi * ai * direction.y * cos(theta);
	}

	return total;
}

float3 CalculateGerstnerNormals(float3 inputVertex,float3 inputNormal)
{
	float3 normal = inputNormal;
	for (int i = 0; i < numWaves; i++)
	{
		Wave wave = waves[i];
		float2 direction = wave.direction;
		float wi = 2 * 3.1416 / wave.wavelength;
		float ai = wave.amplitude;
		float phi = speed * wi;
		float theta = wi * dot(inputVertex.xz, direction) + phi * time;
		float qi = steepness / wi * wave.amplitude * numWaves;

		float WA = wi * ai;
		float S = sin(theta);
		float C = cos(theta);

		normal.x -= direction.x * WA * C;
		normal.y = qi * WA * S;
		normal.z -= direction.y * WA * C;
	}
	normal.y = 1 + normal.y;

	return normalize(normal);
}

float3 ClaculateGerstnerTangents(float3 inputVertex, float3 inputTangents)
{
	float3 tangent = inputTangents;
	for (int i = 0; i < numWaves; i++)
	{
		Wave wave = waves[i];
		float2 direction = wave.direction;
		float wi = 2 * 3.1416 / wave.wavelength;
		float ai = wave.amplitude;
		float phi = speed * wi;
		float theta = wi * dot(inputVertex.xz, direction) + phi * time;
		float qi = steepness / wi * wave.amplitude * numWaves;

		float WA = wi * ai;
		float S = sin(theta);
		float C = cos(theta);

		tangent.x -= qi * direction.x * direction.y * WA * S;
		tangent.y += direction.y * WA * C;
		tangent.z -= qi * direction.y * direction.y * WA * S;
	}
	tangent.y = 1 - tangent.y;

	return normalize(tangent);
}

void WaveEq()
{
	//----------------------------Water Motion-----------------------------------
	//int u = 64;
	//int v = 64;

	//float s_X = 0;
	//float s_Y = 0;
	//float s_Z = 0;

	//output.uv = float2(input.position[0] * 22.0 / 2816.0, input.position[2] * 16.5 / 2112.0);


	//float2 X0 = float2(input.position[0], input.position[2]);
	//float height = 0;
	//for (int i = 0; i < 3; i++)
	//{
	//	float2 K = float2(1, i);				// K is wave vector
	//	float w = 1.5 * (i + 1) / 3.0;			// w is frequency
	//	float a = 0.001;							// a is amplitude
	//	float2 X = X0 - K * a*sin(dot(K, X0) - w * time * 10);
	//	float  y = a * cos(dot(K, X0) - w * time * 10);
	//	s_X += X[0] / 64 - 0.5f;
	//	s_Y += X[1] / 64 - 0.5f;
	//	s_Z += y;
	//	
	//}
	//input.position[0] = s_X;
	//input.position[2] = s_Y;
	//input.position[1] = s_Z; //s_Z
	//
	//
	////---------------------------------------------------------------
	//input.position *= 100.0f;
}
Texture2D displacementMap : register(t4);
SamplerState basicSampler : register(s1);

VertexToHull main(VertexShaderInput input)
{
	VertexToHull output;
	float scaleFactor = 3.0f;
	//input.uv.x += time;
	float displacedHeight = displacementMap.SampleLevel(basicSampler, input.uv,0).x;
	

	// Sine wave
	float height = 1;
	//input.position.y = height * sin(input.position.x + time) * sin(input.position.y + time);

	// Apply Gerstner wave equation
	input.normal = CalculateGerstnerNormals(input.position, input.normal);
	input.tangent = ClaculateGerstnerTangents(input.position, input.tangent);
	input.position = CalculateGerstnerWave(input.position);

	// Transform to world position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	output.uv = input.uv;
	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	
	
	// simple displacement mapping
	//output.position.y += scaleFactor * displacedHeight;

	// output vertex for interpolatio across triangles
	//output.uv = mul(float4(input.uv, 0.0f, 1.0f), gTexTransform).xy;

	// Normalized tessellation factor. 
	// The tessellation is 
	//   0 if d >= gMinTessDistance and
	//   1 if d <= gMaxTessDistance.  
	float tess = saturate((gMinTessDistance) / (gMinTessDistance - gMaxTessDistance));

	// Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
	output.tessFactor = gMinTessFactor + tess * (gMaxTessFactor - gMinTessFactor);
	
	return output;
}
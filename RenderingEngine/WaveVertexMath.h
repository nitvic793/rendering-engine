#pragma once

#include <vector>
#include "SimpleMath.h"
#include "Vertex.h"
#include "Water.h"

using namespace std;

//Copied from CalculateGerstnerWave function from Waver Vertex Shader
XMFLOAT3 CalculateGerstnerWave(XMFLOAT3 input, Wave * waves, int numWaves) {
	float speed = 30;
	XMFLOAT3 total = XMFLOAT3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < numWaves; i++) {
		float wi = 2 * 3.1416 / waves[i].wavelength;
		float ai = waves[i].amplitude;
		float phi = speed * wi;
		//Continue from here
		//XMFLOAT2 direction = 
	}

	return XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void CalculateVertices(XMFLOAT3 offset, Vertex* vertices) {
	for (int i = 0; i < 2500; i++) {
		vertices[i].Position.x += offset.x;
		vertices[i].Position.y += offset.y;
		vertices[i].Position.z += offset.z;
	}
}

//float3 CalculateGerstnerWave(float3 inputVertex)
//{
//	float3 total = float3(0, 0, 0);
//
//	for (int i = 0; i < numWaves; i++)
//	{
//		Wave wave = waves[i];
//		// Wavelength (L): the crest-to-crest distance between waves in world space. Wavelength L relates to frequency w as w = 2/L.
//		float wi = 2 * 3.1416 / wave.wavelength;
//		// Amplitude (A): the height from the water plane to the wave crest.
//		float ai = wave.amplitude;
//		// Speed (S): the distance the crest moves forward per second.
//		// It is convenient to express speed as phase-constant, where phase-constant = S x 2/L
//		float phi = speed * wi;
//		// direction normalized
//		float2 direction = normalize(wave.direction);
//		// Angle calculated
//		// Water mesh is in the xz plane not xy plane as the gerstner equation suggests
//		float theta = wi * dot(inputVertex.xz, direction) + phi * time;
//		// Qi is a parameter that controls the steepness of the waves.
//		//	For a single wave i, Qi of 0 gives the usual rolling sine wave, and Qi = 1/(wi Ai ) gives a sharp crest.
//		//  range of 0 to 1, and using Qi = Q/(wi Ai x numWaves) to vary from totally smooth waves to the sharpest waves
//		float qi = steepness / wi * ai;
//
//		// X and Z values as per gerstner equation
//		total.x += inputVertex.x + qi * ai * direction.x * cos(theta);
//		total.y += wave.amplitude * sin(theta);
//		total.z += inputVertex.z + qi * ai * direction.y * cos(theta);
//	}
//
//	return total;
//}
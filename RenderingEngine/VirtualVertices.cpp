#include "VirtualVertices.h"

VirtualVertices::VirtualVertices() {

}

VirtualVertices::~VirtualVertices() {
	delete startVertices;
	delete finalVertices;
}

void VirtualVertices::SetPosition(Vector3 position) {
	this->position = position;
}

void VirtualVertices::SetVertices(Vertex *vertices, int vertexCount) {
	this->vertexCount = vertexCount;
	this->startVertices = new Vector3[vertexCount];
	this->finalVertices = new Vector3[vertexCount];

	for (int i = 0; i < vertexCount; i++) {
		this->startVertices[i] = vertices[i].Position;
	}
}

Vector2 VirtualVertices::GetXZ(Vector3 input) {
	return Vector2(input.x, input.z);
}

//Most code is copied over from VS_WaterShader.hlsl
Vector3 VirtualVertices::ApplyGerstnerWave(Vector3 inputVertex, Wave *waves, int numWaves, float time)
{
	Vector3 total = Vector3(0, 0, 0);

	for (int i = 0; i < numWaves; i++)
	{
		Wave wave = waves[i];
		// Wavelength (L): the crest-to-crest distance between waves in world space. Wavelength L relates to frequency w as w = 2/L.
		float wi = 2 * 3.1416 / wave.wavelength;
		// Amplitude (A): the height from the water plane to the wave crest.
		float ai = wave.amplitude;
		// Speed (S): the distance the crest moves forward per second.
		// It is convenient to express speed as phase-constant, where phase-constant = S x 2/L
		float phi = SPEED * wi;
		// direction normalized
		Vector2 direction = wave.direction;
		direction.Normalize();
		// Angle calculated
		// Water mesh is in the xz plane not xy plane as the gerstner equation suggests
		float theta = wi * GetXZ(inputVertex).Dot(direction) + phi * time;
		// Qi is a parameter that controls the steepness of the waves.
		//	For a single wave i, Qi of 0 gives the usual rolling sine wave, and Qi = 1/(wi Ai ) gives a sharp crest.
		//  range of 0 to 1, and using Qi = Q/(wi Ai x numWaves) to vary from totally smooth waves to the sharpest waves
		float qi = STEEPNESS / wi * ai;

		// X and Z values as per gerstner equation
		total.x += inputVertex.x + qi * ai * direction.x * cos(theta);
		total.y += wave.amplitude * sin(theta);
		total.z += inputVertex.z + qi * ai * direction.y * cos(theta);
	}

	total += position;

	return total;
}

Vector3 &VirtualVertices::operator[](int index) {
	return finalVertices[index];
}

bool VirtualVertices::HitWater(Vector3 spearTipPosition) {
	for (int i = 0; i < vertexCount; i++) {
		if (Vector3::Distance(spearTipPosition, finalVertices[i]) < DISTANCE && 
			finalVertices[i].y - spearTipPosition.y >= 0 && 
			finalVertices[i].y - spearTipPosition.y <= 0.5f) {
			return true;
		}
	}
	return false;
}

void VirtualVertices::ApplyGetstnerWaves(Wave *waves, int numWaves, float time) {
	for (int i = 0; i < vertexCount; i++) {
		finalVertices[i] = ApplyGerstnerWave(startVertices[i], waves, numWaves, time);
	}
}
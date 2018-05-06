#pragma once

#include <d3d11.h>
#include "SimpleMath.h"
#include "Vertex.h"
#include "Water.h"

using namespace DirectX::SimpleMath;

const float STEEPNESS = 0.9f;
const float SPEED = 20;

class VirtualVertices {
public:
	VirtualVertices();
	~VirtualVertices();
	void SetVertices(Vertex *vertices, int VertexCount);
	void ApplyGetstnerWaves(Wave *waves, int numWaves, float time);
	Vector3 &operator[](int index);
private:
	int vertexCount;
	Vector3 *startVertices;
	Vector3 *finalVertices;
	Vector2 GetXZ(Vector3 input);
	Vector3 ApplyGerstnerWave(Vector3 inputVertex, Wave *waves, int numWaves, float time);
};
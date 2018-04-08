#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Vertex.h"
#include "Mesh.h"
#include <memory>
#include "Entity.h"
#include <random>

#define	NUM_OF_WAVES 100
struct Wave
{
	XMFLOAT2 direction;
	float amplitude;
	float wavelength;
};
class Water
{
private:
	XMFLOAT4X4 reflectionmatrix;
	ID3D11Buffer* reflectionBuffer;	//reflection world matrix
	int length ;
	int breadth ;
	UINT * indices;
	Vertex * vertices;
	Wave waves[NUM_OF_WAVES];
public:
	Water(int _length, int _breadth);
	~Water();
	void GenerateWaterMesh();
	UINT* GetIndices();
	Vertex* GetVertices();
	UINT GetVertexCount();
	UINT GetIndexCount();
	void CalculateUVCoordinates();
	void CreateWaves();
	Wave* GetWaves();
};


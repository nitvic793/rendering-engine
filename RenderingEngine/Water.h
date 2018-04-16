#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Vertex.h"
#include "Mesh.h"
#include <memory>
#include "Entity.h"
#include <random>

#define	NUM_OF_WAVES 10

//------------------------------------------------
// Properties of a single wave
// TODO: Try and add steepness and speed
//------------------------------------------------
struct Wave 
{
	XMFLOAT2 direction;
	float amplitude;
	float wavelength;
};

class Water : public Entity
{
private:
	XMFLOAT4X4 reflectionmatrix;
	ID3D11Buffer* reflectionBuffer;	//reflection world matrix
	int length ;
	int breadth ;
	UINT * indices;
	Vertex * vertices;
	Wave waves[NUM_OF_WAVES];
	
	void GenerateWaterMesh();
	void CalculateUVCoordinates();
public:
	Water(int _length, int _breadth);
	~Water();

	void Init(Material* mat, ID3D11Device * device);
	void CreateWaves();

	// Getters
	UINT*	GetIndices() const;
	Vertex* GetVertices() const;
	UINT	GetVertexCount() const;
	UINT	GetIndexCount() const;
	Wave*	GetWaves() ;
};


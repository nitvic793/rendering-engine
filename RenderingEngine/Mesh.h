#pragma once

#include <d3d11.h>
#include "Vertex.h"
#include <vector>
#include <DirectXMath.h>
#include <fstream>

using namespace DirectX;

struct Triangle
{
	int a, b, c;
	Vertex vA, vB, vC;
};

class Mesh
{
public:
	Mesh();
	Mesh(Vertex *vertices, UINT vertexCount, UINT *indices, UINT indexCount, ID3D11Device *device);
	Mesh(const char* filename, ID3D11Device *device);
	~Mesh();
	ID3D11Buffer *GetVertexBuffer();
	ID3D11Buffer *GetIndexBuffer();
	UINT GetIndexCount();
	void Initialize(Vertex *vertices, UINT vertexCount, UINT *indices, UINT indexCount, ID3D11Device *device);
	void CalculateTangents(Vertex *vertices, UINT vertexCount, UINT *indices, UINT indexCount);
	XMFLOAT3 GetMaxDimensions() const;
	XMFLOAT3 GetMinDimensions() const;
private:
	XMFLOAT3 minDimensions;
	XMFLOAT3 maxDimensions;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer; 
	UINT indexCount;
	std::vector<Triangle> triangles;
};


#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Entity.h"

using namespace DirectX;

const int TEXTURE_REPEAT = 16;

class Terrain : public Entity
{
	int terrainHeight;
	int terrainWidth;
	XMFLOAT3* heightMap;
	XMFLOAT3* heightNormals;
	XMFLOAT2* textureCoords;
	Vertex *vertices;
	UINT *indices;
	int indexCount;
public:
	const int GetTerrainHeight();
	const int GetTerrainWidth();
	void CalculateNormals();
	void CalculateUVCoordinates(); 
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
	bool Initialize(const char* filename, ID3D11Device* device, ID3D11DeviceContext* context);
	void PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix) override;
	Terrain();
	~Terrain();
};


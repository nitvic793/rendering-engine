#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Entity.h"

using namespace DirectX;

const int TEXTURE_REPEAT = 1;

typedef ID3D11ShaderResourceView* DXTexPtr;

class Terrain : public Entity
{
	int terrainHeight;
	int terrainWidth;
	XMFLOAT3* heightMap;
	XMFLOAT3* heightNormals;
	XMFLOAT2* textureCoords;
	VertexTerrain *vertices;
	UINT *indices;
	int indexCount;
	ID3D11ShaderResourceView* splatMap;
	ID3D11ShaderResourceView* redTexture; //Sea bed
	ID3D11ShaderResourceView* blueTexture; //beach
	ID3D11ShaderResourceView* greenTexture; //grass
	ID3D11ShaderResourceView* alphaTexture; //sea bed 2
public:
	const int GetTerrainHeight();
	const int GetTerrainWidth();
	void CalculateNormals();
	void CalculateUVCoordinates(); 
	void SetTextures(DXTexPtr red, DXTexPtr green, DXTexPtr blue, DXTexPtr alpha);
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
	bool Initialize(const char* filename, ID3D11Device* device, ID3D11DeviceContext* context);
	void SetSplatMap(ID3D11ShaderResourceView* splat);
	virtual void PrepareMaterialWithShadows(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4X4 shadowViewMatrix, XMFLOAT4X4 shadowProjectionMatrix, ID3D11SamplerState* shadowSampler, ID3D11ShaderResourceView* shadowSRV) override;
	void PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix) override;
	Terrain();
	~Terrain();
};


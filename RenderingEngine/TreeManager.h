#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

#define MAX_INSTANCE 64

using namespace DirectX;

struct TreeInstanceType
{
	XMFLOAT4X4 world;
};

class TreeManager
{
	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;
	std::vector<XMFLOAT3> positions;
	XMFLOAT4X4* treeInstances;
	int instanceCount;
	ID3D11Buffer* instanceBuffer;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	ID3D11RasterizerState* rasterizer;
	void Render(int index, Camera* camera);
public:
	void InitializeTrees(std::vector<std::string> meshNames, std::vector<std::string> materialNames, std::vector<XMFLOAT3> positionVector);
	void Render(Camera* camera);
	TreeManager(ID3D11Device* device, ID3D11DeviceContext* context);
	~TreeManager();
};


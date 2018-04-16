#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

using namespace DirectX;

struct TreeInstanceType
{
	XMFLOAT3 Position;
};

class TreeManager
{
	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;
	std::vector<XMFLOAT3> positions;
	TreeInstanceType* treeInstances;
	int instanceCount;
	ID3D11Buffer* instanceBuffer;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	void Render(int index, Camera* camera);
public:
	void InitializeTrees(std::vector<std::string> meshNames, std::vector<std::string> materialNames, std::vector<XMFLOAT3> positionVector);
	void Render(Camera* camera);
	TreeManager(ID3D11Device* device, ID3D11DeviceContext* context);
	~TreeManager();
};


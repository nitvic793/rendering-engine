#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"

using namespace DirectX;

class TreeManager
{
	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;
	std::vector<XMFLOAT3> positions;
public:
	void InitializeTrees(std::vector<std::string> meshNames, std::vector<std::string> materialNames, std::vector<XMFLOAT3> positionVector);
	TreeManager();
	~TreeManager();
};


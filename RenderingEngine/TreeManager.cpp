#include "TreeManager.h"
#include "Resources.h"


void TreeManager::InitializeTrees(std::vector<std::string> meshNames, std::vector<std::string> materialNames, std::vector<XMFLOAT3> positionsVector)
{
	auto rm = Resources::GetInstance();
	for (auto mName : meshNames)
	{
		meshes.push_back(rm->meshes[mName]);
	}

	for (auto mName : materialNames)
	{
		materials.push_back(rm->materials[mName]);
	}

	positions = positionsVector;
}

TreeManager::TreeManager()
{
}


TreeManager::~TreeManager()
{
}

#pragma once
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"

using namespace DirectX;
class Entity
{
protected:
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 rotation;

	Mesh *mesh;
	Material* material;
public:
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetScale();
	void SetPosition(XMFLOAT3 pos);
	void SetRotationZ(float angle);
	void SetPosition(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void Move(XMFLOAT3 offset);
	void PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
	void SetLights(std::unordered_map<std::string, DirectionalLight> lights);
	void SetLights(std::unordered_map<std::string, Light*> lights);
	void SetCameraPosition(XMFLOAT3 position);
	Mesh *GetMesh();
	Entity(Mesh *m, Material* mat);
	~Entity();
};


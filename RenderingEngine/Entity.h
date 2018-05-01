#pragma once
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include <DirectXCollision.h>
#include "FBXLoader.h"

using namespace DirectX;
class Entity
{
protected:
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 rotation;
	BoundingOrientedBox boundingBox;
	Mesh *mesh;
	Material* material;
public:
	BoundingOrientedBox GetBoundingBox();
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetScale();
	void RotateX(float angle);
	void RotateY(float angle);
	void SetMaterial(Material* mat);
	void SetPosition(XMFLOAT3 pos);
	void SetRotationZ(float angle);
	void SetRotation(float roll, float pitch, float yaw);
	void SetPosition(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void Move(XMFLOAT3 offset);
	virtual void PrepareMaterialWithShadows(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4X4 shadowViewMatrix, XMFLOAT4X4 shadowProjectionMatrix, ID3D11SamplerState* shadowSampler, ID3D11ShaderResourceView* shadowSRV);
	virtual void PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
	void PrepareMaterialAnimated(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, FBXLoader*);
	void SetLights(std::unordered_map<std::string, DirectionalLight> lights);
	void SetLights(std::unordered_map<std::string, Light*> lights);
	void SetCameraPosition(XMFLOAT3 position);
	virtual void Update(float deltaTime, float totalTime);
	Mesh *GetMesh();
	Material *GetMaterial();
	Entity(Mesh *m, Material* mat);
	~Entity();

	bool hasShadow = true;
	bool isAnimated = false;
};


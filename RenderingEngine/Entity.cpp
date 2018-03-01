#include "Entity.h"

Entity::Entity(Mesh *m, Material* mat)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMVECTOR v = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR sc = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	XMStoreFloat3(&position, v);
	XMStoreFloat3(&scale, sc);
	XMStoreFloat3(&rotation, v);
	mesh = m;
	material = mat;
}

Entity::~Entity()
{
}

XMFLOAT4X4 Entity::GetWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotY = XMMatrixRotationZ(rotation.z);
	XMMATRIX scle = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX world = scle * rotY * trans;
	//world = XMMatrixMultiply(XMMatrixIdentity(), world);
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	return worldMatrix;
}

void Entity::SetPosition(XMFLOAT3 pos)
{
	position = pos;
}

void Entity::SetRotationZ(float angle)
{
	rotation.z = angle;
}

void Entity::SetPosition(float x, float y, float z)
{
	this->position.x = x;
	this->position.y = y;
	this->position.z = z;
}

void Entity::SetScale(float x, float y, float z)
{
	this->scale.x = x;
	this->scale.y = y;
	this->scale.z = z;
}

XMFLOAT3 Entity::GetScale()
{
	return scale;
}

void Entity::Move(XMFLOAT3 offset)
{
	auto pos = XMLoadFloat3(&position);
	auto off = XMLoadFloat3(&offset);
	auto newPos = XMVectorAdd(pos, off);
	XMStoreFloat3(&position, newPos);
}

void Entity::PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	auto vertexShader = material->GetVertexShader();
	auto pixelShader = material->GetPixelShader();
	vertexShader->SetMatrix4x4("world", GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);
	pixelShader->SetSamplerState("basicSampler", material->GetSampler());
	pixelShader->SetShaderResourceView("diffuseTexture", material->GetSRV());
	if(material->GetNormalSRV())
	pixelShader->SetShaderResourceView("normalTexture", material->GetNormalSRV());
	else pixelShader->SetShaderResourceView("normalTexture", nullptr);
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();
}

void Entity::SetLights(std::unordered_map<std::string, DirectionalLight> lights)
{
	auto pixelShader = material->GetPixelShader();
	for (auto lightPair : lights)
	{
		auto light = lightPair.second;
		pixelShader->SetData(lightPair.first, &light, sizeof(DirectionalLight));
	}
}

void Entity::SetLights(std::unordered_map<std::string, Light*> lights)
{
	auto pixelShader = material->GetPixelShader();
	for (auto lightPair : lights)
	{
		auto light = lightPair.second;
		switch(light->Type)
		{ 
		case Directional:
			pixelShader->SetData(lightPair.first, light->GetLight<DirectionalLight>(), sizeof(DirectionalLight));
			break;
		case Point:
			pixelShader->SetData(lightPair.first, light->GetLight<PointLight>(), sizeof(PointLight));
			break;
		}		
	}
}

void Entity::SetCameraPosition(XMFLOAT3 position)
{
	auto pixelShader = material->GetPixelShader();
	pixelShader->SetFloat3("cameraPosition", position);
}

Mesh *Entity::GetMesh()
{
	return mesh;
}

XMFLOAT3 Entity::GetPosition()
{
	return position;
}

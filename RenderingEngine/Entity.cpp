#include "Entity.h"
#include <algorithm>

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
	if (m != nullptr)
	{
		auto minV = m->GetMinDimensions();
		auto maxV = m->GetMaxDimensions();
		XMFLOAT3 center = XMFLOAT3((maxV.x - minV.x) / 2, (maxV.y - minV.y) / 2, (maxV.z - minV.z) / 2);
		boundingBox.Extents = center;
		auto actualCenter = XMVectorAdd(XMLoadFloat3(&center), XMLoadFloat3(&position));
		XMStoreFloat3(&center, actualCenter);
		boundingBox.Center = center;
	}
}

Entity::~Entity()
{
}

BoundingOrientedBox Entity::GetBoundingBox()
{
	auto bBox = boundingBox;
	bBox.Center = position;
	bBox.Extents.x *= scale.x;
	bBox.Extents.y *= scale.y;
	bBox.Extents.z *= scale.z;
	auto rot = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMStoreFloat4(&bBox.Orientation, rot);
	return bBox;
}

XMFLOAT4X4 Entity::GetWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX scle = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX world = scle * rot * trans;
	//world = XMMatrixMultiply(XMMatrixIdentity(), world);
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	return worldMatrix;
}

void Entity::SetPosition(XMFLOAT3 pos)
{
	position = pos;
	boundingBox.Center = position;
}

void Entity::SetRotationZ(float angle)
{
	rotation.z = angle;
}

void Entity::SetRotation(float roll, float pitch, float yaw)
{
	rotation.x = roll;
	rotation.y = pitch;
	rotation.z = yaw;
}

void Entity::SetPosition(float x, float y, float z)
{
	this->position.x = x;
	this->position.y = y;
	this->position.z = z;
	boundingBox.Center = position;
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

void Entity::RotateX(float angle)
{
	rotation.x += angle;
}

void Entity::RotateY(float angle)
{
	rotation.y += angle;
}

void Entity::SetMaterial(Material * mat)
{
	material = mat;
}

void Entity::Move(XMFLOAT3 offset)
{
	auto pos = XMLoadFloat3(&position);
	auto off = XMLoadFloat3(&offset);
	auto newPos = XMVectorAdd(pos, off);
	XMStoreFloat3(&position, newPos);
}

void Entity::PrepareMaterialWithShadows(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4X4 shadowViewMatrix, XMFLOAT4X4 shadowProjectionMatrix, ID3D11SamplerState* shadowSampler, ID3D11ShaderResourceView* shadowSRV)
{
	auto vertexShader = material->GetVertexShader();
	auto pixelShader = material->GetPixelShader();
	//auto vertexShader = vs;
	//auto pixelShader = ps;
	vertexShader->SetMatrix4x4("world", GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);

	vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
	vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);
	pixelShader->SetSamplerState("basicSampler", material->GetSampler());
	pixelShader->SetSamplerState("shadowSampler", shadowSampler);
	pixelShader->SetShaderResourceView("diffuseTexture", material->GetSRV());
	pixelShader->SetShaderResourceView("shadowMapTexture", shadowSRV);
	if (material->GetNormalSRV())
		pixelShader->SetShaderResourceView("normalTexture", material->GetNormalSRV());
	else pixelShader->SetShaderResourceView("normalTexture", nullptr);

	pixelShader->SetShaderResourceView("roughnessTexture", material->GetRoughnessSRV());
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();
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

	if (material->GetNormalSRV())
		pixelShader->SetShaderResourceView("normalTexture", material->GetNormalSRV());
	else
		pixelShader->SetShaderResourceView("normalTexture", nullptr);

	pixelShader->SetShaderResourceView("roughnessTexture", material->GetRoughnessSRV());
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();
}

void Entity::PrepareMaterialAnimated(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, FBXLoader * fbxLoader)
{
	auto vertexShader = material->GetVertexShader();
	auto pixelShader = material->GetPixelShader();

	vertexShader->SetMatrix4x4("world", GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);

	int bonesSize = 0;
	bonesSize = (sizeof(XMFLOAT4X4) * 20 * 2);
	Bones bones[20];

	fbxLoader->GetAnimatedMatrixExtra();

	//Setting bones
	int numBones = fbxLoader->skeleton.mJoints.size();
	for (int i = 0; i < numBones; i++)
	{

		XMMATRIX jointTransformMatrix = XMLoadFloat4x4(&fbxLoader->skeleton.mJoints[i].mTransform);
		XMMATRIX invJointTransformMatrix = XMLoadFloat4x4(&fbxLoader->skeleton.mJoints[i].mGlobalBindposeInverse);

		XMFLOAT4X4 trans = {};
		XMStoreFloat4x4(&trans, XMMatrixTranspose(jointTransformMatrix));
		bones[i].BoneTransform = trans;
		XMFLOAT4X4 trans2 = {};
		XMStoreFloat4x4(&trans2, XMMatrixTranspose(invJointTransformMatrix));
		bones[i].InvBoneTransform = trans2;
	}
	vertexShader->SetData("bones", &bones, bonesSize);

	//pixelShader->SetSamplerState("basicSampler", material->GetSampler());
	//pixelShader->SetShaderResourceView("diffuseTexture", material->GetSRV());
	//pixelShader->SetShaderResourceView("normalTexture", material->GetNormalSRV());
	//pixelShader->SetShaderResourceView("roughnessTexture", material->GetRoughnessSRV());

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
	std::vector<DirectionalLight> dirLights;
	std::vector<PointLight> pointLights;
	std::for_each(lights.begin(), lights.end(), [&](std::pair<const std::string, Light*>  & element) {
		switch (element.second->Type)
		{
		case Directional:
		{
			auto light = *element.second->GetLight<DirectionalLight>();
			dirLights.push_back(light);
			break;
		}
		case Point:
			auto light = *element.second->GetLight<PointLight>();
			pointLights.push_back(light);
			break;
		}
	});
	int dirLightCount = (int)dirLights.size();
	int pointLightCount = (int)pointLights.size();
	pixelShader->SetInt("DirectionalLightCount", dirLightCount);
	pixelShader->SetInt("PointLightCount", pointLightCount);
	pixelShader->SetData("dirLights", dirLights.data(), sizeof(DirectionalLight) * MAX_LIGHTS);
	pixelShader->SetData("pointLights", pointLights.data(), sizeof(PointLight) * MAX_LIGHTS);

	for (auto lightPair : lights)
	{
		auto light = lightPair.second;
		switch (light->Type)
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

void Entity::Update(float deltaTime, float totalTime)
{
}

Mesh *Entity::GetMesh()
{
	return mesh;
}

Material * Entity::GetMaterial()
{
	return material;
}

XMFLOAT3 Entity::GetPosition()
{
	return position;
}

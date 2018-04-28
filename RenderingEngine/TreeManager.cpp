#include "TreeManager.h"
#include "Resources.h"


void TreeManager::Render(int index, Camera * camera)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = sizeof(Vertex);
	strides[1] = sizeof(XMFLOAT4X4);
	offsets[0] = 0;
	offsets[1] = 0;
	bufferPointers[0] = meshes[index]->GetVertexBuffer();
	bufferPointers[1] = instanceBuffer;
	context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	context->IASetIndexBuffer(meshes[index]->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto mat = materials[index];
	auto ps = mat->GetPixelShader();
	auto vs = mat->GetVertexShader();
	auto sampler = mat->GetSampler();

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMMatrixIdentity()));

	vs->SetMatrix4x4("world", world);
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());;

	ps->SetSamplerState("basicSampler", mat->GetSampler());
	ps->SetShaderResourceView("diffuseTexture", mat->GetSRV());
	ps->SetShaderResourceView("normalTexture", mat->GetNormalSRV());
	ps->SetShaderResourceView("roughnessTexture", mat->GetRoughnessSRV());

	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	vs->SetShader();
	ps->SetShader();
	context->RSSetState(rasterizer);
	context->DrawInstanced(meshes[index]->GetVertexCount(), instanceCount, 0, 0);
	context->RSSetState(nullptr);
}

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

	auto identityMat = XMMatrixIdentity();
	positions = positionsVector;
	instanceCount = (int)positions.size();
	treeInstances = new XMFLOAT4X4[instanceCount];
	for (int i = 0; i < instanceCount; ++i)
	{
		auto instaMat = identityMat * XMMatrixScaling(1, 1, 1) * XMMatrixRotationZ(0)* XMMatrixTranslationFromVector(XMLoadFloat3(&positions[i]));
		XMStoreFloat4x4(&treeInstances[i], XMMatrixTranspose(instaMat));
		//XMStoreFloat4x4(&worldInstances[i], XMMatrixTranspose(instaMat));
		//treeInstances[i].world = positions[i];
	}

	D3D11_BUFFER_DESC instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA instanceData;
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(XMFLOAT4X4) * instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	instanceData.pSysMem = (void*)treeInstances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
}

void TreeManager::Render(Camera* camera)
{
	for (int i = 0; i < meshes.size(); ++i)
		Render(i, camera);
}

TreeManager::TreeManager(ID3D11Device* device, ID3D11DeviceContext* context)
{
	this->device = device;
	this->context = context;
	instanceBuffer = nullptr;
	treeInstances = nullptr;
	D3D11_RASTERIZER_DESC  rasDesc = {};
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&rasDesc, &rasterizer);
}


TreeManager::~TreeManager()
{
	if (instanceBuffer)instanceBuffer->Release();
	if (treeInstances)delete[] treeInstances;
	if (rasterizer)rasterizer->Release();
}

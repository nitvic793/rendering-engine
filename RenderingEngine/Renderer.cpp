#include "Renderer.h"

void Renderer::SetShadowViewProj(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection, ID3D11SamplerState* sampler, ID3D11ShaderResourceView* srv)
{
	shadowViewMatrix = view;
	shadowProjectionMatrix = projection;
	shadowSampler = sampler;
	shadowSRV = srv;
}

void Renderer::SetDepthStencilView(ID3D11DepthStencilView * depthStencilView)
{
	this->depthStencilView = depthStencilView;
}

void Renderer::SetResources(Resources* rsrc)
{
	resources = rsrc;
}

void Renderer::ClearScreen(const float color[4])
{
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
}

void Renderer::SetCamera(Camera * cam)
{
	camera = cam;
}

void Renderer::SetLights(std::unordered_map<std::string, Light*> lightsMap)
{
	lights = lightsMap;
}

void Renderer::Draw(Entity* entity)
{
	if (!entity->isAnimated)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		entity->SetCameraPosition(camera->GetPosition());
		entity->SetLights(lights);
		if (entity->hasShadow)
			entity->PrepareMaterialWithShadows(camera->GetViewMatrix(), camera->GetProjectionMatrix(), shadowViewMatrix, shadowProjectionMatrix, shadowSampler, shadowSRV);
		else
			entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
		auto mesh = entity->GetMesh();
		auto vertexBuffer = mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);
	}
	else
	{
		UINT stride = sizeof(VertexAnimated);
		UINT offset = 0;
		entity->SetCameraPosition(camera->GetPosition());
		entity->SetLights(lights);

		entity->PrepareMaterialAnimated(camera->GetViewMatrix(), camera->GetProjectionMatrix(), &resources->fishFBX);


		auto mesh = entity->GetMesh();
		auto vertexBuffer = mesh->GetVertexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);

		int a = 1;
	}

}

void Renderer::Draw(Terrain * entity)
{
	UINT stride = sizeof(VertexTerrain);
	UINT offset = 0;
	entity->SetCameraPosition(camera->GetPosition());
	entity->SetLights(lights);
	if (entity->hasShadow)
		entity->PrepareMaterialWithShadows(camera->GetViewMatrix(), camera->GetProjectionMatrix(), shadowViewMatrix, shadowProjectionMatrix, shadowSampler, shadowSRV);
	else
		entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
	auto mesh = entity->GetMesh();
	auto vertexBuffer = mesh->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);
}

void Renderer::DrawAsLineList(Entity * entity)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	entity->SetCameraPosition(camera->GetPosition());
	entity->SetLights(lights);
	entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
	auto mesh = entity->GetMesh();
	auto vertexBuffer = mesh->GetVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); //Set to Line list
	context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //Reset to triangle list
}

void Renderer::Present()
{
	swapChain->Present(0, 0);
}

Renderer::Renderer(ID3D11DeviceContext *ctx, 
	ID3D11RenderTargetView *backBuffer, 
	ID3D11DepthStencilView *depthStencil,
	IDXGISwapChain *inSwapChain):
	context(ctx),
	backBufferRTV(backBuffer),
	depthStencilView(depthStencil),
	swapChain(inSwapChain)
{	
	depthStencilView = depthStencil;
}

Renderer::~Renderer()
{
}
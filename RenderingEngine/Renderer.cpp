#include "Renderer.h"

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

void Renderer::DrawEntity(Entity* entity)
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
	context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);
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
}

Renderer::~Renderer()
{
}

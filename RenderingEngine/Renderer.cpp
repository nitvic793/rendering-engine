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

void Renderer::DrawEntity(Entity* entity)
{
	if (!entity->isAnimated)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		entity->SetCameraPosition(camera->GetPosition());
		entity->SetLights(lights);
		if(entity->hasShadow)
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

#pragma region Render To texture test

//ID3D11Texture2D* renderTargetTextureMap;
//ID3D11RenderTargetView* renderTargetViewMap;
//ID3D11ShaderResourceView* shaderResourceViewMap;
//void Renderer::RenderToTexture(ID3D11Device* device, ID3D11DeviceContext*	context)
//{
//	ID3D11Texture2D* renderTargetTextureMap;
//	ID3D11RenderTargetView* renderTargetViewMap;
//	ID3D11ShaderResourceView* shaderResourceViewMap;
//
//	// Our map camera's view and projection matrices
//	//XMMATRIX mapView;
//	//XMMATRIX mapProjection;
//
//	D3D11_TEXTURE2D_DESC textureDesc;
//	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
//	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
//
//	///////////////////////// Map's Texture
//	// Initialize the  texture description.
//	ZeroMemory(&textureDesc, sizeof(textureDesc));
//
//	// Setup the texture description.
//	// We will have our map be a square
//	// We will need to have this texture bound as a render target AND a shader resource
//	textureDesc.Width = 500;
//	textureDesc.Height = 500;
//	textureDesc.MipLevels = 1;
//	textureDesc.ArraySize = 1;
//	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
//	textureDesc.SampleDesc.Count = 1;
//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
//	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//	textureDesc.CPUAccessFlags = 0;
//	textureDesc.MiscFlags = 0;
//
//	// Create the texture
//	device->CreateTexture2D(&textureDesc, NULL, &renderTargetTextureMap);
//
//	/////////////////////// Map's Render Target
//	// Setup the description of the render target view.
//	renderTargetViewDesc.Format = textureDesc.Format;
//	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
//	renderTargetViewDesc.Texture2D.MipSlice = 0;
//
//	// Create the render target view.
//	device->CreateRenderTargetView(renderTargetTextureMap, &renderTargetViewDesc, &renderTargetViewMap);
//
//	/////////////////////// Map's Shader Resource View
//	// Setup the description of the shader resource view.
//	shaderResourceViewDesc.Format = textureDesc.Format;
//	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
//	shaderResourceViewDesc.Texture2D.MipLevels = 1;
//
//	// Create the shader resource view.
//	device->CreateShaderResourceView(renderTargetTextureMap, &shaderResourceViewDesc, &shaderResourceViewMap);
//
//	////////////////////////// Draw Terrain Onto Map
//	// Here we will draw our map, which is just the terrain from the mapCam's view
//
//	// Set our maps Render Target
//	context->OMSetRenderTargets(1, &renderTargetViewMap, depthStencilView);
//
//	// Now clear the render target
//	const float color[4] = { 0.11f, 0.11f, 0.11f, 0.0f };
//	context->ClearRenderTargetView(renderTargetViewMap, color);
//}
//
//void Renderer::RenderReflection(Entity* entity)
//{
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	entity->SetCameraPosition(camera->GetPosition());
//	entity->SetLights(lights);
//	camera->RenderReflectionMatrix(10);
//	entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
//	auto mesh = entity->GetMesh();
//	auto vertexBuffer = mesh->GetVertexBuffer();
//	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
//	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
//	context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);
//}
//
//Material* reflMat;
//Water * water;
//Mesh* ms;
//Entity* entity;
//
//void Renderer::createwater(ID3D11Device* device, ID3D11SamplerState* sampler, SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader)
//{
//	reflMat = new Material(vertexShader, pixelShader, shaderResourceViewMap, sampler);
//	water = new Water(64, 64);
//	water->GenerateWaterMesh();
//	//vertexShader->SetFloat("time", time);
//	ms = new Mesh(water->GetVertices(), water->GetVertexCount(), water->GetIndices(), water->GetIndexCount(), device);
//
//	entity = new Entity(ms, reflMat);
//}
//void Renderer::DrawReflectionTexture()
//{
//	// Reset the context
//	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
//
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	entity->SetCameraPosition(camera->GetPosition());
//	entity->SetLights(lights);
//	camera->RenderReflectionMatrix(10);
//	entity->PrepareMaterial(camera->GetViewMatrix(), camera->GetProjectionMatrix());
//	auto mesh = entity->GetMesh();
//	auto vertexBuffer = mesh->GetVertexBuffer();
//	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
//	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
//	context->DrawIndexed((UINT)mesh->GetIndexCount(), 0, 0);
//}

#pragma endregion
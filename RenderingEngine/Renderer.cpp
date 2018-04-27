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

void Renderer::RenderToTexture(ID3D11Device* device, ID3D11DeviceContext*	context)
{
	// Refraction setup ------------------------
	ID3D11Texture2D* reflectionRenderTexture;

	// Set up render texture
	D3D11_TEXTURE2D_DESC rtDesc = {};
	rtDesc.Width = width;
	rtDesc.Height = height;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtDesc.Usage = D3D11_USAGE_DEFAULT;
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = 0;
	rtDesc.MiscFlags = 0;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.SampleDesc.Quality = 0;
	device->CreateTexture2D(&rtDesc, 0, &reflectionRenderTexture);


	// Set up render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = rtDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(reflectionRenderTexture, &rtvDesc, &reflectionRTV);

	// Set up shader resource view for same texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(reflectionRenderTexture, &srvDesc, &reflectionSRV);

	// All done with this texture ref
	reflectionRenderTexture->Release();

	// Set up a sampler that uses clamp addressing
	// for use when doing refration - this is useful so 
	// that we don't wrap the refraction from the other
	// side of the screen
	D3D11_SAMPLER_DESC rSamp = {};
	rSamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	rSamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	rSamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	rSamp.Filter = D3D11_FILTER_ANISOTROPIC;
	rSamp.MaxAnisotropy = 16;
	rSamp.MaxLOD = D3D11_FLOAT32_MAX;

	// Set up a blend state
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	bd.RenderTarget[0].BlendEnable = true;

	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&bd, &blendState);



	// Ask DirectX for the actual object
	device->CreateSamplerState(&rSamp, &reflectionSampler);

	//////////////////////////////
	// Shadow data initialization
	//////////////////////////////

	int reflectionSize = 2048;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC reflectionDesc = {};
	reflectionDesc.Width = reflectionSize;
	reflectionDesc.Height = reflectionSize;
	reflectionDesc.ArraySize = 1;
	reflectionDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	reflectionDesc.CPUAccessFlags = 0;
	reflectionDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	reflectionDesc.MipLevels = 1;
	reflectionDesc.MiscFlags = 0;
	reflectionDesc.SampleDesc.Count = 1;
	reflectionDesc.SampleDesc.Quality = 0;
	reflectionDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* reflectionTexture;
	device->CreateTexture2D(&reflectionDesc, 0, &reflectionTexture);

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC reflectionDSDesc = {};
	reflectionDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	reflectionDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	reflectionDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(reflectionTexture, &reflectionDSDesc, &reflectionDSV);

	// Create the SRV for the shadow map
	srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(reflectionTexture, &srvDesc, &reflectionSRV); //..................................................

	// Release the texture reference since we don't need it
	reflectionTexture->Release();

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC reflectionSampDesc = {};
	reflectionSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	reflectionSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	reflectionSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	reflectionSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	reflectionSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	reflectionSampDesc.BorderColor[0] = 1.0f;
	reflectionSampDesc.BorderColor[1] = 1.0f;
	reflectionSampDesc.BorderColor[2] = 1.0f;
	reflectionSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&reflectionSampDesc, &reflectionSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC reflectionRastDesc = {};
	reflectionRastDesc.FillMode = D3D11_FILL_SOLID;
	reflectionRastDesc.CullMode = D3D11_CULL_BACK;
	reflectionRastDesc.DepthClipEnable = true;
	reflectionRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	reflectionRastDesc.DepthBiasClamp = 0.0f;
	reflectionRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&reflectionRastDesc, &reflectionRasterizer);

	camera->RenderReflectionMatrix(1.0f);
	reflectionViewMatrix = camera->GetReflectionMatrix();
}

void Camera::RenderReflectionMatrix(float height)
{
	XMFLOAT3 up, reflPosition, lookAt;
	float radians;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.
	// For planar reflection invert the Y position of the camera.
	reflPosition.x = position.x;
	reflPosition.y = -position.y + (height * 2.0f);
	reflPosition.z = position.z;

	// Calculate the rotation in radians.
	radians = rotationY * 0.0174532925f;

	// Setup where the camera is looking.
	lookAt.x = sinf(radians) + position.x;
	lookAt.y = reflPosition.y;
	lookAt.z = cosf(radians) + position.z;

	// Create the view matrix from the three vectors.
	XMStoreFloat4x4(&reflectionMatrix, XMMatrixLookAtLH(XMLoadFloat3(&reflPosition), XMLoadFloat3(&lookAt), XMLoadFloat3(&up)));
	XMStoreFloat4x4(&reflectionMatrix, XMMatrixTranspose(XMLoadFloat4x4(&reflectionMatrix)));
	return;
}
#include "Game.h"
#include "Vertex.h"
#include "WaveVertexMath.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1366,			   // Width of the window's client area
		768,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	camera = nullptr;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	for (auto entity : entities)
	{
		delete entity;
	}
	entities.clear();

	for (auto model : models)
	{
		delete model.second;
	}

	for (auto lights : lightsMap) {
		delete lights.second;
	}

	/*for (auto ripple : ripples) {
		delete ripple;
	}*/

	lightsMap.clear();
	models.clear();
	delete vertexShader;
	delete pixelShader;
	delete camera;
	delete renderer;
	delete resources;
	delete shadowVS;

	skyDepthState->Release();
	sampler->Release();
	skyRastState->Release();

	refractSampler->Release();
	refractionRTV->Release();
	refractionSRV->Release();

	blendState->Release();

	shadowDSV->Release();
	shadowSRV->Release();
	shadowSampler->Release();;
	shadowRasterizer->Release();

	displacementSampler->Release();
	delete currentProjectile;
	delete water;
	delete hullShader;
	delete domainShader;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	projectileHitWater = false;
	RECT rect;
	GetWindowRect(this->hWnd, &rect);
	prevMousePos.x = width / 2;
	prevMousePos.y = height / 2 - 30;
	SetCursorPos(rect.left + width / 2, rect.top + height / 2);
	resources = new Resources(device, context, swapChain);
	resources->LoadResources();

	LoadShaders();
	CreateCamera();
	InitializeEntities();
	InitializeRenderer();

	// Create a sampler state that holds options for sampling
	// The descriptions should always just be local variables
	D3D11_SAMPLER_DESC samplerDesc = {}; // The {} part zeros out the struct!
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Setting this allows for mip maps to work! (if they exist)

											// Ask DirectX for the actual object
	device->CreateSamplerState(&samplerDesc, &sampler);

	// Create states for sky rendering
	D3D11_RASTERIZER_DESC rs = {};
	rs.CullMode = D3D11_CULL_FRONT;
	rs.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&rs, &skyRastState);

	D3D11_DEPTH_STENCIL_DESC ds = {};
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);

	// Refraction setup ------------------------
	ID3D11Texture2D* refractionRenderTexture;

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
	device->CreateTexture2D(&rtDesc, 0, &refractionRenderTexture);


	// Set up render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = rtDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(refractionRenderTexture, &rtvDesc, &refractionRTV);

	// Set up shader resource view for same texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(refractionRenderTexture, &srvDesc, &refractionSRV);

	// All done with this texture ref
	refractionRenderTexture->Release();

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
	device->CreateSamplerState(&rSamp, &refractSampler);

	//////////////////////////////
	// Shadow data initialization
	//////////////////////////////

	shadowMapSize = 2048;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &shadowDSDesc, &shadowDSV);

	// Create the SRV for the shadow map
	srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, &shadowSRV);

	// Release the texture reference since we don't need it
	shadowTexture->Release();

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"PreShadowVS.cso");

	hullShader = new SimpleHullShader(device, context);
	hullShader->LoadShaderFile(L"TesselationHullShader.cso");

	domainShader = new SimpleDomainShader(device, context);
	domainShader->LoadShaderFile(L"TesselationDomainShader.cso");
}

// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateCamera()
{
	camera = new Camera((float)width / height);
}

void Game::CreateWater()
{
	time = 0.0f;
	translate = 0.0f;
	water = new Water(50,50);
	water->Init(resources->materials["water"],device);
	water->SetPosition(-125, -6, -150);
	//waterbject->SetScale(3, 3, 3);
	water->CreateWaves();
	resources->vertexShaders["water"]->SetData("waves", water->GetWaves(), sizeof(Wave) * NUM_OF_WAVES);

#pragma region Displacement Mapping Disabled
	//------------------------------- Displacement map test-----------------------------------
	//Load Sampler
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &displacementSampler);

	resources->vertexShaders["water"]->SetShaderResourceView("displacementMap", resources->shaderResourceViews["waterDisplacement"]);
	resources->vertexShaders["water"]->SetSamplerState("basicSampler", displacementSampler);
	//------------------------------- Displacement map test-----------------------------------
#pragma endregion
}



void Game::RenderEntityShadow(Entity * entity)
{
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* vb = entity->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = entity->GetMesh()->GetIndexBuffer();

	// Set buffers in the input assembler
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	// Finish setting shadow-creation VS stuff
	shadowVS->SetMatrix4x4("world", entity->GetWorldMatrix());
	shadowVS->CopyAllBufferData();

	// Finally do the actual drawing
	context->DrawIndexed(entity->GetMesh()->GetIndexCount(), 0, 0);
}

void Game::RenderShadowMap()
{
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, 10, 1, 0),	// Start back and in the air
		XMVectorSet(0, 0, 0, 0),	// Look at the origin
		XMVectorSet(0, 1, 0, 0));	// Up is up
	XMStoreFloat4x4(&shadowViewMatrix, XMMatrixTranspose(shView));

	XMMATRIX shProj = XMMatrixOrthographicLH(20.0f, 20.0f, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, XMMatrixTranspose(shProj));

	ID3D11RenderTargetView * nullRTV = NULL;
	context->OMSetRenderTargets(1, &nullRTV, NULL);
	ID3D11ShaderResourceView *const nullSRV[3] = { NULL };
	context->PSSetShaderResources(0, 3, nullSRV);

	// Setup the initial states for shadow map creation
	context->OMSetRenderTargets(0, 0, shadowDSV);
	context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer);

	// We need a viewport that matches the shadow map resolution
	D3D11_VIEWPORT shadowVP = {};
	shadowVP.TopLeftX = 0;
	shadowVP.TopLeftY = 0;
	shadowVP.Width = (float)shadowMapSize;
	shadowVP.Height = (float)shadowMapSize;
	shadowVP.MinDepth = 0.0f;
	shadowVP.MaxDepth = 1.0f;
	context->RSSetViewports(1, &shadowVP);

	// Set up the shadow-creation Vertex Shader
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);

	// Turn OFF the pixel shader
	context->PSSetShader(0, 0, 0);
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		RenderEntityShadow(entities[i]);
	}
	RenderEntityShadow(currentProjectile);

	//shadowDSV = nullptr;
	context->OMSetRenderTargets(1, &nullRTV, NULL);
	context->PSSetShaderResources(0, 3, nullSRV);

	// Revert any render state changes
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	shadowVP.Width = (float)this->width;
	shadowVP.Height = (float)this->height;
	context->RSSetViewports(1, &shadowVP);

	renderer->SetShadowViewProj(shadowViewMatrix, shadowProjectionMatrix, shadowSampler, shadowSRV);
}

void Game::InitializeEntities()
{
	ShowCursor(false);
	trees = std::unique_ptr<TreeManager>(new TreeManager(device, context));
	fishes = std::unique_ptr<FishController>(new FishController(
		resources->meshes["Rudd-Fish_Cube.001"],
		resources->materials["fish"],
		25,
		XMFLOAT3(9.f, -8.5f, -15.f),
		XMFLOAT3(9.f, -8.5f, 20.f),
		5,
		XMFLOAT3(0, 90.f * XM_PI / 180, 0),
		XMFLOAT3(0.03f, 0.03f, 0.03f)
	));
	trees->InitializeTrees({ "palm","palm_2" }, { "palm","palm_2" }, { XMFLOAT3(-30,-6,0), XMFLOAT3(-25,-7,0) });
	terrain = std::unique_ptr<Terrain>(new Terrain());
	terrain->Initialize("../../Assets/Terrain/heightmap.bmp", device, context);
	terrain->SetMaterial(resources->materials["grass"]);

	terrain->SetPosition(-125, -10.5, -150);
	light.AmbientColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 0);
	light.DiffuseColor = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.f);
	light.Direction = XMFLOAT3(1.f, 0, 0.f);

	secondaryLight.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	secondaryLight.DiffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1);
	secondaryLight.Direction = XMFLOAT3(0, -1, 0);

	pointLight.Color = XMFLOAT4(0.0f, 0.f, 0.f, 1);
	pointLight.Position = XMFLOAT3(0.4f, 2.f, -14.9f);
	pointLight.Range = 20.f;

	lightsMap.insert(std::pair<std::string, Light*>("light", new Light{ &light, Directional }));
	lightsMap.insert(std::pair<std::string, Light*>("secondaryLight", new Light{ &secondaryLight, Directional }));
	lightsMap.insert(std::pair<std::string, Light*>("pointLight", new Light{ &pointLight, Point }));

	currentProjectile = new ProjectileEntity(resources->meshes["spear"], resources->materials["spear"]);
	currentProjectile->SetRotation(180 * XM_PI / 180, 0, 90 * XM_PI / 180);
	currentProjectile->SetPosition(0.4f, 2.f, -14.9f);
	currentProjectile->SetScale(1.5f, 1.5f, 1.5f);

	entities.push_back(new Entity(resources->meshes["sphere"], resources->materials["metal"]));
	entities.push_back(new Entity(resources->meshes["boat"], resources->materials["boat"]));
	entities.push_back(new Entity(resources->meshes["Rudd-Fish_Cube.001"], resources->materials["fish"]));
	entities.push_back(new Entity(resources->meshes["palm"], resources->materials["palm"]));
	entities.push_back(new Entity(resources->meshes["palm_2"], resources->materials["palm_2"]));
	entities.push_back(new Entity(resources->meshes["tuna"], resources->materials["tuna"]));
	//entities.push_back(new Entity(resources->meshes["Coconut_Tree"], resources->materials["boat"]));

	CreateWater();
	entities[0]->SetPosition(1.f, 1.f, 1.f);

	entities[1]->SetScale(0.6f, 0.6f, 0.6f);
	entities[1]->SetPosition(0.f, -5.5f, 0.f);
	entities[1]->SetRotation(0, 180.f * XM_PI / 180, 0);
	entities[2]->SetScale(0.03f, 0.03f, 0.03f);
	entities[2]->SetPosition(9.f, -8.5f, -15.f);
	entities[2]->SetRotation(0, 90.f * XM_PI / 180, 0);

	entities[3]->SetPosition(-24.f, -6.f, -4.f);
	entities[4]->SetPosition(-24.f, -6.f, -4.f);

	entities[3]->SetScale(0.5f, 0.5f, 0.5f);
	entities[4]->SetScale(0.5f, 0.5f, 0.5f);
	//entities[2]->hasShadow = false;
}

void Game::InitializeRenderer()
{
	renderer = new Renderer(context, backBufferRTV, depthStencilView, swapChain);
	renderer->SetCamera(camera);
	renderer->SetLights(lightsMap);
	renderer->SetResources(resources);
}

void Game::DrawRefraction()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* vb = water->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = water->GetMesh()->GetIndexBuffer();
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	auto refractVS = resources->vertexShaders["refraction"];
	auto refractPS = resources->pixelShaders["refraction"];
	// Setup vertex shader
	refractVS->SetMatrix4x4("world", water->GetWorldMatrix());
	refractVS->SetMatrix4x4("view", camera->GetViewMatrix());
	refractVS->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	refractVS->SetFloat("time", time);
	refractVS->SetData("waves", water->GetWaves(), sizeof(Wave) * NUM_OF_WAVES);
	refractVS->CopyAllBufferData();
	refractVS->SetShader();

	// Setup pixel shader
	refractPS->SetShaderResourceView("ScenePixels", refractionSRV);	// Pixels of the screen
	refractPS->SetShaderResourceView("NormalMap", water->GetMaterial()->GetNormalSRV());	// Normal map for the object itself
	refractPS->SetSamplerState("BasicSampler", sampler);			// Sampler for the normal map
	refractPS->SetSamplerState("RefractSampler", refractSampler);	// Uses CLAMP on the edges
	refractPS->SetFloat3("CameraPosition", camera->GetPosition());
	refractPS->SetMatrix4x4("view", camera->GetViewMatrix());				// View matrix, so we can put normals into view space
	refractPS->CopyAllBufferData();
	refractPS->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(water->GetMesh()->GetIndexCount(), 0, 0);
}

void Game::DrawFullscreenQuad(ID3D11ShaderResourceView * texture)
{
	// First, turn off our buffers, as we'll be generating the vertex
	// data on the fly in a special vertex shader using the index of each vert
	context->IASetVertexBuffers(0, 0, 0, 0, 0);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	auto quadVS = resources->vertexShaders["quad"];
	auto quadPS = resources->pixelShaders["quad"];
	// Set up the fullscreen quad shaders
	quadVS->SetShader();

	quadPS->SetShaderResourceView("Pixels", texture);
	quadPS->SetSamplerState("Sampler", sampler);
	quadPS->SetShader();

	// Draw
	context->Draw(3, 0);
}

void Game::CreateRipple(float x, float y, float z, float duration, float ringSize) {
	Ripple r = Ripple(x, y, z, duration, ringSize);
	ripples.push_back(r);
}

//----------------------------------------------------
// Set the shaders and draw water
//----------------------------------------------------
void Game::DrawWater()
{
	// Set water shaders
	resources->vertexShaders["water"]->SetFloat("time", time);
	resources->pixelShaders["water"]->SetFloat("translate", translate);
	resources->pixelShaders["water"]->SetShaderResourceView("SkyTexture", resources->shaderResourceViews["cubemap"]);
	resources->pixelShaders["water"]->SetShaderResourceView("normalTextureTwo", resources->shaderResourceViews["waterNormal2"]);
	resources->pixelShaders["water"]->SetFloat("transparency", transparency);

	// Setup pixel shader
	resources->pixelShaders["water"]->SetShaderResourceView("ScenePixels", refractionSRV);	// Pixels of the screen
	resources->pixelShaders["water"]->SetSamplerState("RefractSampler", refractSampler);	// Uses CLAMP on the edges
	resources->pixelShaders["water"]->SetFloat3("CameraPosition", camera->GetPosition());
	resources->pixelShaders["water"]->SetMatrix4x4("view", camera->GetViewMatrix());		// View matrix, so we can put normals into view space
	resources->pixelShaders["water"]->CopyAllBufferData();
	renderer->DrawEntity(water);
}
// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	renderer->SetDepthStencilView(depthStencilView);
	camera->SetProjectionMatrix((float)width / height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//Water Position: -125, -6, -150
	//std::cout << water->GetVertices()[40].Position.x << std::endl;
	//std::cout << currentProjectile->GetPosition().x + 125 << std::endl;
	// Water .........................................
	time += 0.05f * deltaTime;
	translate += 0.1f * deltaTime;
	if (translate > 1.0f)
	{
		translate -= 1.0f;
	}

	fishes->Update(deltaTime, totalTime);
	float fishSpeed = 2.f;
	entities[2]->Move(XMFLOAT3((sin(totalTime * 3) / 600), 0, fishSpeed*deltaTime));

	if (entities[2]->GetPosition().z >= 30.f)
	{
		entities[2]->SetPosition(9.f, -8.5f, -15.f);
	}

	//resources->vertexShaders["water"]->SetFloat("time", time); 
	//resources->pixelShaders["water"]->SetFloat("translate", translate);
	//.................................................

	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
	{
		projectilePreviousPosition = currentProjectile->GetPosition();
		currentProjectile->Shoot(1.f, camera->GetDirection());
	}

	if ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) {
		CreateRipple(0.0f, 0.0f, 50.0f, 2.0f, 2.0f);
	}

	/*if (fishes->CheckForCollision(currentProjectile))
	{
		printf("Hit!");
	}*/

	//Check for spear hitting the water
	if (currentProjectile->GetPosition().y <= -7.0f && !projectileHitWater) {
		projectileHitWater = true;
		XMFLOAT3 pos = currentProjectile->GetPosition();
		float x = pos.x;
		float z = pos.z;
		CreateRipple(x, 0.0f, z, 2.0f, 0.5f);
	}

	auto distance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&currentProjectile->GetPosition()) - XMLoadFloat3(&camera->GetPosition())));

	if (fabsf(distance) > 50 || fishes->CheckForCollision(currentProjectile))
	{
		projectileHitWater = false;
		currentProjectile->SetHasBeenShot(false);
		//currentProjectile->SetPosition(0.4f, 2.f, -14.9f);
		auto pos = camera->GetPosition();
		pos = XMFLOAT3(pos.x + 0.4f, pos.y, pos.z);
		currentProjectile->SetPosition(pos);
	}

	//Update Camera
	camera->Update(deltaTime);
	for (auto entity : entities)
	{
		entity->Update(deltaTime, totalTime);
	}
	//currentProjectile->SetPosition(camera->GetPosition());
	currentProjectile->Update(deltaTime, totalTime);

	//Update entities
	entities[1]->SetRotation(cos(totalTime) / 20, 180.f * XM_PI / 180, -sin(totalTime) / 20);

	//Update ripples and Water shader (add support for multiple ripples later)
	//Delete ripples afterward if they are at max duration

	for (auto it = ripples.begin(); it != ripples.end(); ) {
		(*it).Update(deltaTime);
		if ((*it).AtMaxDuration()) {
			it = ripples.erase(it);
			//delete (*it);
		}
		else {
			it++;
		}
	}

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	RenderShadowMap();

	const float color[4] = { 0.11f, 0.11f, 0.11f, 0.0f };
	renderer->ClearScreen(color);

	// Clear any and all render targets we intend to use, and the depth buffer
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(refractionRTV, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Use our refraction render target and our regular depth buffer
	context->OMSetRenderTargets(1, &refractionRTV, depthStencilView);

	renderer->DrawEntity(terrain.get());
	renderer->DrawEntity(entities[2]);
	fishes->Render(renderer);
	DrawSky();
	//context->OMSetBlendState(blendState, 0, 0xFFFFFFFF);

	// Reset blend state if blending
	
	context->OMSetBlendState(0, 0, 0xFFFFFFFF);
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	DrawFullscreenQuad(refractionSRV);

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	DrawWater();
	renderer->DrawEntity(currentProjectile);

	for (auto entity : entities)
	{
		if (entity->hasShadow)
			renderer->DrawEntity(entity);
	}
	trees->Render(camera);

	ID3D11ShaderResourceView *const nullSRV[4] = { NULL };
	context->PSSetShaderResources(0, 4, nullSRV);

	for (auto entity : entities)
	{
		if (!entity->hasShadow)
			renderer->DrawEntity(entity);
	}

	context->PSSetShaderResources(0, 4, nullSRV);
	ID3D11ShaderResourceView* nullSRV2[16] = {};
	context->PSSetShaderResources(0, 16, nullSRV2);

	//Reset water if there are no ripples
	resources->pixelShaders["water"]->SetFloat3("ripplePosition", XMFLOAT3(0.0f, 0.0f, 0.0f));
	resources->pixelShaders["water"]->SetFloat("rippleRadius", 0.0f);
	resources->pixelShaders["water"]->SetFloat("ringSize", 0.0f);
	
	//Convert Ripples to RippleData structs, then
	//Pass ripples to the water shader
	std::vector<RippleData> rippleData;
	for (auto ripple : ripples) {
		rippleData.push_back(ripple.GetRippleData());
	}
	if (rippleData.size() > 0) {
		resources->pixelShaders["water"]->SetData("ripples", rippleData.data(), sizeof(RippleData) * MAX_RIPPLES);
	}
	resources->pixelShaders["water"]->SetInt("rippleCount", (int)ripples.size());

	renderer->Present();
}

void Game::Tesellation()
{
#pragma region Tesselation
	// --- tessellation test ---
	//vertexShader->SetFloat("gMaxTessDistance", 5);
	//vertexShader->SetFloat("gMinTessDistance", 2);
	//vertexShader->SetFloat("gMinTessFactor", 2);
	//vertexShader->SetFloat("gMaxTessFactor", 3);
	//domainShader->SetMatrix4x4("view", camera->GetViewMatrix());
	//domainShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	//domainShader->SetShaderResourceView("heightSRV", resources->shaderResourceViews["waterDisplacement"]);
	//domainShader->SetSamplerState("heightSampler", displacementSampler);
	//hullShader->SetShader();
	//domainShader->SetShader();
	//hullShader->CopyAllBufferData();
	//domainShader->CopyAllBufferData();
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	//renderer->DrawEntity(entities[2]);

	// -- end tessellation --
#pragma endregion
}

void Game::DrawSky()
{
	// After I draw any and all opaque entities, I want to draw the sky
	ID3D11Buffer* skyVB = resources->meshes["cube"]->GetVertexBuffer();
	ID3D11Buffer* skyIB = resources->meshes["cube"]->GetIndexBuffer();


	// Set the buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Set up the sky shaders
	resources->vertexShaders["sky"]->SetMatrix4x4("view", camera->GetViewMatrix());
	resources->vertexShaders["sky"]->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	resources->vertexShaders["sky"]->CopyAllBufferData();
	resources->vertexShaders["sky"]->SetShader();

	resources->pixelShaders["sky"]->SetShaderResourceView("SkyTexture", resources->shaderResourceViews["cubemap"]);
	resources->pixelShaders["sky"]->SetSamplerState("BasicSampler", sampler);
	resources->pixelShaders["sky"]->SetShader();

	// Set up the render states necessary for the sky
	context->RSSetState(skyRastState);
	context->OMSetDepthStencilState(skyDepthState, 0);
	context->DrawIndexed(resources->meshes["cube"]->GetIndexCount(), 0, 0);


	// When done rendering, reset any and all states for the next frame
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{

	//if (buttonState & 0x0001)
	//{
	float speed = 0.6f;
	float deltaX = (float)x - prevMousePos.x;
	float deltaY = (float)y - prevMousePos.y;
	camera->RotateX(speed * deltaY * XM_PI / 180);
	camera->RotateY(speed * deltaX * XM_PI / 180);

	currentProjectile->RotateY(speed * deltaX * XM_PI / 180);
	currentProjectile->RotateX(speed * deltaY * XM_PI / 180);
	//}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	if (buttonState & 0x0001)
	{

	}
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion
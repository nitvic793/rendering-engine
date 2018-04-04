#include "Game.h"
#include "Vertex.h"

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

	lightsMap.clear();
	models.clear();
	delete vertexShader;
	delete pixelShader;
	delete camera;
	delete renderer;
	delete resources;

	skyDepthState->Release();
	sampler->Release();
	skyRastState->Release();

	delete currentProjectile;
	delete water;
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
	RECT rect;
	GetWindowRect(this->hWnd, &rect);
	prevMousePos.x = width / 2;
	prevMousePos.y = height / 2 - 30;
	SetCursorPos(rect.left + width / 2, rect.top + height / 2);
	resources = new Resources(device, context);
	resources->LoadResources();

	LoadShaders();
	CreateCamera();
	InitializeEntities();
	InitializeRenderer();


	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



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
	water = new Water(1000, 1000);
	water->GenerateWaterMesh();
	water->CalculateUVCoordinates();
	resources->vertexShaders["water"]->SetFloat("time", time);
	resources->pixelShaders["water"]->SetShaderResourceView("SkyTexture", resources->shaderResourceViews["cubemap"]);
	models.insert(std::pair<std::string, Mesh*>("quad", new Mesh(water->GetVertices(), water->GetVertexCount(), water->GetIndices(), water->GetIndexCount(), device)));
	waterObject = new Entity(models["quad"], resources->materials["water"]);
	waterObject->SetPosition(-125, -7, -150);
	entities.push_back(waterObject);
}

void Game::InitializeEntities()
{
	ShowCursor(false);

	terrain = std::unique_ptr<Terrain>(new Terrain());
	terrain->Initialize("../../Assets/Terrain/heightmap.bmp", device, context);
	terrain->SetMaterial(resources->materials["grass"]);

	terrain->SetPosition(-125, -8.5, -150);
	light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	light.DiffuseColor = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.f);
	light.Direction = XMFLOAT3(1.f, 0, 0.f);

	secondaryLight.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	secondaryLight.DiffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1);
	secondaryLight.Direction = XMFLOAT3(0, -1, 0);

	pointLight.Color = XMFLOAT4(0.6f, 0.6f, 0.6f, 1);
	pointLight.Position = XMFLOAT3(0, 0, 0);
	pointLight.Range = 20.f;

	lightsMap.insert(std::pair<std::string, Light*>("light", new Light{ &light, Directional }));
	lightsMap.insert(std::pair<std::string, Light*>("secondaryLight", new Light{ &secondaryLight, Directional }));
	lightsMap.insert(std::pair<std::string, Light*>("pointLight", new Light{ &pointLight, Point }));

	currentProjectile = new ProjectileEntity(resources->meshes["spear"], resources->materials["spear"]);
	currentProjectile->SetRotation(180 * XM_PI / 180, 0, 90 * XM_PI / 180);
	currentProjectile->SetPosition(0.4f, 2.f, -14.9f);
	currentProjectile->SetScale(1.5f, 1.5f, 1.5f);
	//entities.push_back(new Entity(resources->meshes["spear"], resources->materials["spear"]));
	entities.push_back(new Entity(resources->meshes["sphere"], resources->materials["metal"]));
	//entities.push_back(new Entity(resources->meshes["helix"], resources->materials["wood"]));
	//entities.push_back(new Entity(resources->meshes["torus"], resources->materials["fabric"]));
	entities.push_back(new Entity(resources->meshes["boat"], resources->materials["boat"]));

	CreateWater();

	entities[0]->SetPosition(1.f, 1.f, 1.9f);
	//entities[0]->SetRotation(180 * XM_PI / 180, 0, 90 * XM_PI / 180);
	entities[1]->SetScale(0.6f, 0.6f, 0.6f);
	entities[1]->SetPosition(-2.f, -7.f, -7.f);
	entities[1]->SetRotation(0, 180.f * XM_PI / 180, 0);
}

void Game::InitializeRenderer()
{
	renderer = new Renderer(context, backBufferRTV, depthStencilView, swapChain);
	renderer->SetCamera(camera);
	renderer->SetLights(lightsMap);
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
	// Water .........................................
	time += 0.001f;
	translate += 0.001f;
	if (translate > 1.0f)
	{
		translate -= 1.0f;
	}

	resources->vertexShaders["water"]->SetFloat("time", time);
	resources->pixelShaders["water"]->SetFloat("translate", translate);
	resources->pixelShaders["water"]->SetFloat("translate", translate);
	//.................................................

	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
	{
		projectilePreviousPosition = currentProjectile->GetPosition();
		currentProjectile->Shoot(0.6f, camera->GetDirection());
	}

	if (currentProjectile->GetBoundingBox().Intersects(entities[0]->GetBoundingBox()))
	{
		printf("Hit!");
	}

	auto distance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&currentProjectile->GetPosition()) - XMLoadFloat3(&camera->GetPosition())));

	if (fabsf(distance) > 50)
	{
		currentProjectile->SetHasBeenShot(false);
		//currentProjectile->SetRotation(180 * XM_PI / 180, 0, 90 * XM_PI / 180);
		currentProjectile->SetPosition(0.4f, 2.f, -14.9f);
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

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	const float color[4] = { 0.11f, 0.11f, 0.11f, 0.0f };
	renderer->ClearScreen(color);
	renderer->DrawEntity(terrain.get());
	for (auto entity : entities)
	{
		renderer->DrawEntity(entity);
	}
	renderer->DrawEntity(currentProjectile);


	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// After I draw any and all opaque entities, I want to draw the sky
	ID3D11Buffer* skyVB = resources->meshes["cube"]->GetVertexBuffer();
	ID3D11Buffer* skyIB = resources->meshes["cube"]->GetIndexBuffer();
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

	renderer->Present();
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
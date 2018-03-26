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
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;
	material = nullptr;
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
	delete material;
	delete fabricMaterial;
	delete woodMaterial;
	delete vertexShader;
	delete pixelShader;
	delete camera;
	delete renderer;
	metalSRV->Release();
	fabricSRV->Release();
	woodSRV->Release();
	metalNormalSRV->Release();
	fabricNormalSRV->Release();
	woodNormalSRV->Release();
	sampler->Release();
	
	waterSRV->Release();
	waterNormalSRV->Release();
	delete waterMaterial;
	delete vertexShaderWater;
	delete pixelShaderWater;
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
	LoadShaders();
	CreateCamera();
	InitializeEntities();
	InitializeRenderer();
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
	vertexShaderWater = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");
	vertexShaderWater->LoadShaderFile(L"VS_WaterShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShaderWater = new SimplePixelShader(device, context);
	pixelShaderWater->LoadShaderFile(L"PS_WaterShader.cso");
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
	//-------------Single quad test------------------------
	//int quadsize = 10;
	//Vertex vertices[4] = {};
	//// Load the vertex array with data.
	//vertices[0].Position = XMFLOAT3(-quadsize, 0.0f, -quadsize);

	//vertices[1].Position = XMFLOAT3(-quadsize, +0.0f, +quadsize);

	//vertices[2].Position = XMFLOAT3(+quadsize, +0.0f, +quadsize);

	//vertices[3].Position = XMFLOAT3(+quadsize, +0.0f, -quadsize);
	//
	//vertices[0].Normal = XMFLOAT3(0, 1.0f, 0);

	//vertices[1].Normal = XMFLOAT3(0, 1.0f, 0);

	//vertices[2].Normal = XMFLOAT3(0, 1.0f, 0);

	//vertices[3].Normal = XMFLOAT3(0, 1.0f, 0);
	//UINT indices[] = { 0, 1, 2, 0, 2, 3 };
	//models.insert(std::pair<std::string, Mesh*>("quad", new Mesh(vertices, 4, indices, 6, device)));
	//-------------Single quad test------------------------
	//
	time = 0.0f;
	translate = 0.0f;
	water = new Water(64,64);
	water->GenerateWaterMesh();
	water->CalculateUVCoordinates();
	vertexShaderWater->SetFloat("time",time);
	models.insert(std::pair<std::string, Mesh*>("quad", new Mesh(water->GetVertices(),water->GetVertexCount(),water->GetIndices(), water->GetIndexCount(),device)));
	entities.push_back(new Entity(models["quad"], waterMaterial));
}

void Game::InitializeEntities()
{
	
	light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	light.DiffuseColor = XMFLOAT4(0.4f, 0.4f, 0.9f, 1.f);
	light.Direction = XMFLOAT3(1.f, 0, 0.f);

	secondaryLight.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	secondaryLight.DiffuseColor = XMFLOAT4(0.9f, 0.4f, 0.4f, 1);
	secondaryLight.Direction = XMFLOAT3(0, -1, 0);

	pointLight.Color = XMFLOAT4(0.4f, 0.9f, 0.4f, 1);
	pointLight.Position = XMFLOAT3(0, 1, 0);

	lightsMap.insert(std::pair<std::string, Light*>("light", new Light{ &light, Directional }));
	lightsMap.insert(std::pair<std::string, Light*>("secondaryLight", new Light{ &secondaryLight, Directional }));
	lightsMap.insert(std::pair<std::string, Light*>("pointLight", new Light{ &pointLight, Point }));

	lights.insert(std::pair<std::string, DirectionalLight>("light", light));
	lights.insert(std::pair<std::string, DirectionalLight>("secondaryLight", secondaryLight));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/metal.jpg", nullptr, &metalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/metalNormal.png", nullptr, &metalNormalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/fabric.jpg", nullptr, &fabricSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/fabricNormal.png", nullptr, &fabricNormalSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/wood.jpg", nullptr, &woodSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/woodNormal.png", nullptr, &woodNormalSRV);
	
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterColor.png", nullptr, &waterSRV);
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterNormal.png", nullptr, &waterNormalSRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	material = new Material(vertexShader, pixelShader, metalSRV, metalNormalSRV, sampler);
	fabricMaterial = new Material(vertexShader, pixelShader, fabricSRV, fabricNormalSRV, sampler);
	woodMaterial = new Material(vertexShader, pixelShader, woodSRV, woodNormalSRV, sampler);
	
	waterMaterial = new Material(vertexShaderWater, pixelShaderWater, waterSRV, waterNormalSRV, sampler);

	models.insert(std::pair<std::string, Mesh*>("sphere", new Mesh("../../Assets/Models/sphere.obj", device)));
	models.insert(std::pair<std::string, Mesh*>("cone", new Mesh("../../Assets/Models/cone.obj", device)));
	models.insert(std::pair<std::string, Mesh*>("cylinder", new Mesh("../../Assets/Models/cylinder.obj", device)));
	models.insert(std::pair<std::string, Mesh*>("cube", new Mesh("../../Assets/Models/cube.obj", device)));
	models.insert(std::pair<std::string, Mesh*>("helix", new Mesh("../../Assets/Models/helix.obj", device)));
	models.insert(std::pair<std::string, Mesh*>("torus", new Mesh("../../Assets/Models/torus.obj", device)));
	
	//----------------------------------------------------------------
	CreateWater();
	//----------------------------------------------------------------

	entities.push_back(new Entity(models["cube"], woodMaterial));
	entities.push_back(new Entity(models["sphere"], material));
	entities.push_back(new Entity(models["helix"], woodMaterial));
	entities.push_back(new Entity(models["torus"], material));
	entities.push_back(new Entity(models["cylinder"], fabricMaterial));
	
	entities[0]->SetPosition(3.f, 0.f, 2.f);
	entities[1]->SetPosition(0.f, 3.f, 0.f);
	entities[2]->SetPosition(0.f, -3.f, 0.f);
	entities[3]->SetPosition(-3.f, 0.f, 0.f);
	entities[4]->SetPosition(-3.f, 3.f, 0.f);

	
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
	camera->SetProjectionMatrix((float)width / height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	time += 0.001f;
	translate += 0.001f;
	if (translate > 1.0f)
	{
		translate -= 1.0f;
	}

	vertexShaderWater->SetFloat("time", time);
	pixelShaderWater->SetFloat("translate", translate);
	//Update Camera
	camera->Update(deltaTime);
	//Update entities
	entities[1]->SetRotationZ(sin(totalTime));
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	XMFLOAT3 offset(0, 0, 0);
	float speed = 10.f * deltaTime;

	//Move entity 2 with arrow keys
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		offset.y += speed;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		offset.y -= speed;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		offset.x -= speed;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		offset.x += speed;
	entities[1]->Move(offset);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	const float color[4] = { 0.11f, 0.11f, 0.11f, 0.0f };
	renderer->ClearScreen(color);
	for (auto entity : entities)
	{
		renderer->DrawEntity(entity);
	}
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
	if (buttonState & 0x0001)
	{
		float speed = 0.1f;
		float deltaX = (float)x - prevMousePos.x;
		float deltaY = (float)y - prevMousePos.y;
		camera->RotateX(speed * deltaY * XM_PI / 180);
		camera->RotateY(speed * deltaX * XM_PI / 180);
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
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
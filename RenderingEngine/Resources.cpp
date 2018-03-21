#include "Resources.h"

Resources* Resources::mInstance = nullptr;

Resources * Resources::GetInstance()
{
	return mInstance;
}

void Resources::LoadResources()
{
	//Ideally all below resources should be read off of a config file and loaded from there. 

	//Load Textures
	ID3D11ShaderResourceView *srv;
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/metal.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("metal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/metalNormal.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("metalNormal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/fabric.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("fabric", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/fabricNormal.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("fabricNormal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/wood.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("wood", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/woodNormal.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("woodNormal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/grass01.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("grass", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/grass01_n.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("grassNormal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/spear.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("spear", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/spearNormal.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("spearNormal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/default.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("default", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/defaultNormal.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("defaultNormal", srv));
	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/defaultSpecular.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("defaultSpecular", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/metalSpecular.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("metalSpecular", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/boattex.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("boat", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/boattexnm.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("boatNormal", srv));

	//Load Sampler
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	//Load Shaders
	auto vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");
	vertexShaders.insert(VertexShaderMapType("default",vertexShader));

	auto pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");
	pixelShaders.insert(PixelShaderMapType("default",pixelShader));

	//Load Materials
	materials.insert(MaterialMapType("metal", new Material(vertexShader, pixelShader, shaderResourceViews["metal"], shaderResourceViews["metalNormal"], shaderResourceViews["metalSpecular"], sampler)));
	materials.insert(MaterialMapType("fabric", new Material(vertexShader, pixelShader, shaderResourceViews["fabric"], shaderResourceViews["fabricNormal"], sampler)));
	materials.insert(MaterialMapType("wood", new Material(vertexShader, pixelShader, shaderResourceViews["wood"], shaderResourceViews["woodNormal"], sampler)));
	materials.insert(MaterialMapType("grass", new Material(vertexShader, pixelShader, shaderResourceViews["grass"], shaderResourceViews["grassNormal"], sampler)));
	materials.insert(MaterialMapType("spear", new Material(vertexShader, pixelShader, shaderResourceViews["spear"], shaderResourceViews["spearNormal"], sampler)));
	materials.insert(MaterialMapType("boat", new Material(vertexShader, pixelShader, shaderResourceViews["boat"], shaderResourceViews["boatNormal"], sampler)));

	//Load Meshes
	meshes.insert(std::pair<std::string, Mesh*>("sphere", new Mesh("../../Assets/Models/sphere.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("cone", new Mesh("../../Assets/Models/cone.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("cylinder", new Mesh("../../Assets/Models/cylinder.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("cube", new Mesh("../../Assets/Models/cube.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("helix", new Mesh("../../Assets/Models/helix.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("torus", new Mesh("../../Assets/Models/torus.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("spear", new Mesh("../../Assets/Models/spear.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("boat", new Mesh("../../Assets/Models/boat.obj", device)));
}

Resources::Resources(ID3D11Device *device, ID3D11DeviceContext *context)
{
	this->device = device;
	this->context = context;
	mInstance = this;
}


Resources::~Resources()
{
	for (auto it : meshes)delete it.second;
	for (auto it : materials)delete it.second;
	for (auto it : shaderResourceViews)it.second->Release();
	for (auto it : pixelShaders)delete it.second;
	for (auto it : vertexShaders)delete it.second;
	sampler->Release();
}

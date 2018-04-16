#include "Resources.h"
#include "DDSTextureLoader.h"
#include "ObjLoader.h"
#include <locale>
#include <codecvt>
#include <string>

std::wstring to_wstring(std::string narrow)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::string narrow = converter.to_bytes(wide_utf16_source_string);
	return converter.from_bytes(narrow);
}

Resources* Resources::mInstance = nullptr;

Vertex MapObjlToVertex(objl::Vertex vertex)
{
	auto pos = XMFLOAT3(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
	auto normal = XMFLOAT3(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
	auto uv = XMFLOAT2(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y);
	return { pos, normal, uv };
}

std::vector<Vertex> MapObjlToVertex(std::vector<objl::Vertex> vertices)
{
	std::vector<Vertex> verts;
	for (auto v : vertices)
	{
		verts.push_back(MapObjlToVertex(v));
	}
	return verts;
}

ID3D11ShaderResourceView* LoadSRV(std::wstring filename, ID3D11Device* device)
{
	ID3D11ShaderResourceView* srv = nullptr;
	if (filename.find(L".dds") != std::string::npos)
	{
		CreateDDSTextureFromFile(device, filename.c_str(), nullptr, &srv);
	}
	else
	{
		CreateWICTextureFromFile(device, filename.c_str(), nullptr, &srv);
	}

	return srv;
}

void AddToMeshMap(objl::Loader loader, MeshMap& map, ID3D11Device* device, std::string prefix, SRVMap& texMap)
{
	std::wstring baseTexAddress = L"../../Assets/Textures/";
	for (auto mesh : loader.LoadedMeshes)
	{
		auto verts = MapObjlToVertex(mesh.Vertices);
		auto indices = mesh.Indices;
		Mesh* m = new Mesh(verts.data(), verts.size(), indices.data(), indices.size(), device);
		map.insert(MeshMapType(prefix + mesh.MeshName, m));
		auto texURI = baseTexAddress + to_wstring(mesh.MeshMaterial.map_Kd);
		texMap.insert(SRVMapType(prefix + mesh.MeshName, LoadSRV(texURI, device)));
	}
}

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

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/grass01_h.jpg", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("grassSpecular", srv));
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

	CreateDDSTextureFromFile(device, L"../../Assets/Textures/SunnyCubeMap.dds", 0, &srv);
	shaderResourceViews.insert(SRVMapType("cubemap", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterColor.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("waterColor", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterNormal2.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("waterNormal2", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/waterNormal.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("waterNormal", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/Heightmaptest.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("waterDisplacement", srv));

	CreateWICTextureFromFile(device, context, L"../../Assets/Textures/tuna.png", nullptr, &srv);
	shaderResourceViews.insert(SRVMapType("tuna", srv));

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
	vertexShaders.insert(VertexShaderMapType("default", vertexShader));

	auto pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");
	pixelShaders.insert(PixelShaderMapType("default", pixelShader));

	auto skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"SkyVS.cso");
	vertexShaders.insert(VertexShaderMapType("sky", skyVS));

	auto skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"SkyPS.cso");
	pixelShaders.insert(PixelShaderMapType("sky", skyPS));

	auto waterVS = new SimpleVertexShader(device, context);
	waterVS->LoadShaderFile(L"VS_WaterShader.cso");
	vertexShaders.insert(VertexShaderMapType("water", waterVS));

	auto waterPS = new SimplePixelShader(device, context);
	waterPS->LoadShaderFile(L"PS_WaterShader.cso");
	pixelShaders.insert(PixelShaderMapType("water", waterPS));

	// Refraction shaders
	auto quadVS = new SimpleVertexShader(device, context);
	quadVS->LoadShaderFile(L"FullscreenQuadVS.cso");
	vertexShaders.insert(VertexShaderMapType("quad", quadVS));

	auto quadPS = new SimplePixelShader(device, context);
	quadPS->LoadShaderFile(L"FullscreenQuadPS.cso");
	pixelShaders.insert(PixelShaderMapType("quad", quadPS));

	auto refractVS = new SimpleVertexShader(device, context);
	refractVS->LoadShaderFile(L"RefractVS.cso");
	vertexShaders.insert(VertexShaderMapType("refraction", refractVS));

	auto refractPS = new SimplePixelShader(device, context);
	refractPS->LoadShaderFile(L"RefractPS.cso");
	pixelShaders.insert(PixelShaderMapType("refraction", refractPS));

	auto shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"VS_Shadow.cso");
	vertexShaders.insert(VertexShaderMapType("shadow", shadowVS));

	auto shadowPS = new SimplePixelShader(device, context);
	shadowPS->LoadShaderFile(L"PS_Shadow.cso");
	pixelShaders.insert(PixelShaderMapType("shadow", shadowPS));

	auto preShadowVS = new SimpleVertexShader(device, context);
	preShadowVS->LoadShaderFile(L"PreShadowVS.cso");
	vertexShaders.insert(VertexShaderMapType("preShadow", preShadowVS));

	//Load Materials
	materials.insert(MaterialMapType("metal", new Material(shadowVS, shadowPS, shaderResourceViews["metal"], shaderResourceViews["metalNormal"], shaderResourceViews["metalSpecular"], sampler)));
	materials.insert(MaterialMapType("fabric", new Material(vertexShader, pixelShader, shaderResourceViews["fabric"], shaderResourceViews["fabricNormal"], sampler)));
	materials.insert(MaterialMapType("wood", new Material(vertexShader, pixelShader, shaderResourceViews["wood"], shaderResourceViews["woodNormal"], sampler)));
	materials.insert(MaterialMapType("grass", new Material(vertexShader, pixelShader, shaderResourceViews["grass"], shaderResourceViews["grassNormal"], shaderResourceViews["grassSpecular"], sampler)));
	materials.insert(MaterialMapType("spear", new Material(vertexShader, pixelShader, shaderResourceViews["spear"], shaderResourceViews["spearNormal"], sampler)));
	materials.insert(MaterialMapType("boat", new Material(shadowVS, shadowPS, shaderResourceViews["boat"], shaderResourceViews["boatNormal"], sampler)));
	materials.insert(MaterialMapType("water", new Material(waterVS, waterPS, shaderResourceViews["waterColor"], shaderResourceViews["waterNormal"], sampler)));
	materials.insert(MaterialMapType("tuna", new Material(vertexShader, pixelShader, shaderResourceViews["tuna"], shaderResourceViews["defaultNormal"], sampler)));

	//Load Meshes
	meshes.insert(std::pair<std::string, Mesh*>("sphere", new Mesh("../../Assets/Models/sphere.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("cone", new Mesh("../../Assets/Models/cone.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("cylinder", new Mesh("../../Assets/Models/cylinder.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("cube", new Mesh("../../Assets/Models/cube.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("helix", new Mesh("../../Assets/Models/helix.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("torus", new Mesh("../../Assets/Models/torus.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("spear", new Mesh("../../Assets/Models/spear.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("boat", new Mesh("../../Assets/Models/boat.obj", device)));
	meshes.insert(std::pair<std::string, Mesh*>("tuna", new Mesh("../../Assets/Models/tuna.obj", device)));

	objl::Loader loader;
	loader.LoadFile("../../Assets/Models/palm_tree.obj");
	AddToMeshMap(loader, meshes, device, "palm", shaderResourceViews);
	materials.insert(MaterialMapType("palm", new Material(vertexShader, pixelShader, shaderResourceViews["palm"], shaderResourceViews["defaultNormal"], sampler)));
	materials.insert(MaterialMapType("palm_2", new Material(vertexShader, pixelShader, shaderResourceViews["palm_2"], shaderResourceViews["defaultNormal"], sampler)));

}

Resources::Resources(ID3D11Device *device, ID3D11DeviceContext *context, IDXGISwapChain* swapChain)
{
	this->device = device;
	this->context = context;
	this->swapChain = swapChain;
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

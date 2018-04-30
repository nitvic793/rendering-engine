#pragma once

#include "Mesh.h"
#include "Material.h"
#include <map>
#include <string>
#include <d3d11.h>
#include "WICTextureLoader.h"
#include "SimpleShader.h"


//Map pair types
typedef std::pair<std::string, Mesh*> MeshMapType;
typedef std::pair<std::string, Material*> MaterialMapType;
typedef std::pair<std::string, ID3D11ShaderResourceView*> SRVMapType;
typedef std::pair<std::string, SimpleVertexShader*> VertexShaderMapType;
typedef std::pair<std::string, SimplePixelShader*> PixelShaderMapType;

//Map types
typedef std::map<std::string, SimpleVertexShader*> VertexShaderMap;
typedef std::map<std::string, SimplePixelShader*> PixelShaderMap;
typedef std::map<std::string, ID3D11ShaderResourceView*> SRVMap;
typedef std::map<std::string, Mesh*> MeshMap;
typedef std::map<std::string, Material*> MaterialMap;

class Resources
{
	static Resources* mInstance;
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	IDXGISwapChain* swapChain;
public:
	ID3D11SamplerState *sampler;
	MeshMap meshes;
	MaterialMap materials;
	SRVMap shaderResourceViews;
	VertexShaderMap vertexShaders;
	PixelShaderMap pixelShaders;
	static Resources* GetInstance();
	void LoadResources();
	ID3D11ShaderResourceView* GetSRV(std::string name);
	Resources(ID3D11Device *device, ID3D11DeviceContext *context, IDXGISwapChain* swapChain);
	~Resources();
};


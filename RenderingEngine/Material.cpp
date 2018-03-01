#include "Material.h"



Material::Material()
{
}

Material::Material(SimpleVertexShader *vertexShader, SimplePixelShader *pixelShader)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
}

Material::Material(SimpleVertexShader *vertexShader, SimplePixelShader *pixelShader, ID3D11ShaderResourceView *srv, ID3D11SamplerState *samplerState)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	textureSRV = srv;
	sampler = samplerState;
	normalSRV = nullptr;
}

Material::Material(SimpleVertexShader *vertexShader, SimplePixelShader *pixelShader, ID3D11ShaderResourceView *srv, ID3D11ShaderResourceView *normal, ID3D11SamplerState *samplerState)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	textureSRV = srv;
	sampler = samplerState;
	normalSRV = normal;
}


Material::~Material()
{
}

SimpleVertexShader * Material::GetVertexShader()
{
	return vertexShader;
}

SimplePixelShader * Material::GetPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView * Material::GetSRV()
{
	return textureSRV;
}

ID3D11ShaderResourceView * Material::GetNormalSRV()
{
	return normalSRV;
}

ID3D11SamplerState * Material::GetSampler()
{
	return sampler;
}

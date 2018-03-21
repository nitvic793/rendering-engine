#pragma once

#include "SimpleShader.h"

class Material
{
	SimpleVertexShader*			vertexShader;
	SimplePixelShader*			pixelShader;
	ID3D11ShaderResourceView*	textureSRV;
	ID3D11ShaderResourceView*	normalSRV;
	ID3D11ShaderResourceView*	roughnessSRV;
	ID3D11SamplerState*			sampler;
public:
	Material();
	Material(SimpleVertexShader*, SimplePixelShader*);
	Material(SimpleVertexShader*, SimplePixelShader*, ID3D11ShaderResourceView*, ID3D11SamplerState*);
	Material(SimpleVertexShader*, SimplePixelShader*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, ID3D11SamplerState*);
	Material(SimpleVertexShader*, SimplePixelShader*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, ID3D11SamplerState*);
	~Material();

	SimpleVertexShader*			GetVertexShader();
	SimplePixelShader*			GetPixelShader();
	ID3D11ShaderResourceView*	GetSRV();
	ID3D11ShaderResourceView*	GetNormalSRV();
	ID3D11ShaderResourceView*	GetRoughnessSRV();
	ID3D11SamplerState*			GetSampler();
};


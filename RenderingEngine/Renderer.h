#pragma once

#include <d3d11.h>
#include "Camera.h"
#include "Lights.h"
#include <unordered_map>
#include "Entity.h"
#include "Water.h"
class Renderer
{
	ID3D11DepthStencilView *depthStencilView;
	ID3D11RenderTargetView *backBufferRTV;
	ID3D11DeviceContext *context;
	IDXGISwapChain *swapChain;
	Camera *camera;
	std::unordered_map<std::string, Light*> lights;
public:
	void SetDepthStencilView(ID3D11DepthStencilView *depthStencilView);
	void ClearScreen(const float color[4]);
	void SetCamera(Camera* cam);
	void SetLights(std::unordered_map<std::string, Light*> lightsMap);
	void DrawEntity(Entity *entity);
	void DrawAsLineList(Entity *entity);
	void Present();
	void RenderToTexture(ID3D11Device* device,ID3D11DeviceContext*	context);
	void RenderReflection(Entity* entity);
	void DrawReflectionTexture();
	void createwater(ID3D11Device* device, ID3D11SamplerState* sampler, SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader);
	Renderer(ID3D11DeviceContext *ctx, ID3D11RenderTargetView *backBuffer, ID3D11DepthStencilView *depthStencil, IDXGISwapChain *inSwapChain);
	~Renderer();
};
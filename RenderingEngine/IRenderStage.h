#pragma once
#include <d3d11.h>
#include "Camera.h"
#include "Lights.h"
#include <unordered_map>
#include "Entity.h"
#include "Water.h"
#include "Resources.h"

class IRenderStage
{
	ID3D11DepthStencilView *depthStencilView;
	ID3D11RenderTargetView *backBufferRTV;
	ID3D11DeviceContext *context;
	IDXGISwapChain *swapChain;
	Camera *camera;
	std::unordered_map<std::string, Light*> lights;
	Resources* resources;
	ID3D11RenderTargetView *renderTarget;
public:
	virtual void Initialize() = 0;
	virtual void Render() = 0;
	IRenderStage();
	~IRenderStage();
};


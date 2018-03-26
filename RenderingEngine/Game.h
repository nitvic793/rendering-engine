#pragma once

#include <memory>
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Entity.h"
#include "Material.h"
#include "Camera.h"
#include "Lights.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "Renderer.h"
#include "Terrain.h"
#include "Resources.h"
#include "ProjectileEntity.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:
	void LoadShaders(); 
	void CreateCamera();
	void InitializeEntities();
	void InitializeRenderer();

	SimpleVertexShader*			vertexShader;
	SimplePixelShader*			pixelShader;
	Camera*		camera;
	std::unique_ptr<Terrain> terrain;

	POINT prevMousePos;
	Renderer *renderer;
	Resources *resources;

	DirectionalLight light;
	DirectionalLight secondaryLight;
	PointLight pointLight;

	ProjectileEntity *currentProjectile;
	XMFLOAT3 projectilePreviousPosition;
	XMFLOAT3 projectilePreviousRotation;
	std::unordered_map<std::string, Light*> lightsMap;
	std::unordered_map<std::string, Mesh*> models;
	std::vector<Entity*> entities;	

	ID3D11SamplerState* sampler;

	ID3D11ShaderResourceView* skySRV;
	ID3D11RasterizerState* skyRastState;
	ID3D11DepthStencilState* skyDepthState;
};


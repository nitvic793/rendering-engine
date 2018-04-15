#pragma once

#define MAX_RIPPLES 32

#include <memory>
#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <iostream>
#include "Mesh.h"
#include "Entity.h"
#include "Material.h"
#include "Camera.h"
#include "Lights.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "Renderer.h"
#include "Ripple.h"
#include "Terrain.h"
#include "Resources.h"
#include "ProjectileEntity.h"
#include "Water.h"

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
	void DrawSky();

	Entity* refractionEntity;

	void DrawRefraction();
	void DrawFullscreenQuad(ID3D11ShaderResourceView* texture);

	void CreateRipple(float x, float y, float z, float duration, float ringSize);
	bool projectileHitWater;
	std::vector<Ripple> ripples;

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
	ID3D11SamplerState* displacementSampler;

	ID3D11BlendState* blendState;

	ID3D11ShaderResourceView* skySRV;
	ID3D11RasterizerState* skyRastState;
	ID3D11DepthStencilState* skyDepthState;

	ID3D11SamplerState* refractSampler;
	ID3D11RenderTargetView* refractionRTV;
	ID3D11ShaderResourceView* refractionSRV;

	// An SRV is good enough for loading textures with the DirectX Toolkit and then
	// using them with shaders 
	ID3D11ShaderResourceView* textureSRV;
	ID3D11ShaderResourceView* normalMapSRV;

	float time, translate;
	float transparency = 0.7f;
	Water * water;
	Entity * waterObject;
	void CreateWater();

	// Shadow data
	void RenderEntityShadow(Entity* entity);
	void RenderShadowMap();
	int shadowMapSize;
	ID3D11DepthStencilView* shadowDSV;
	ID3D11ShaderResourceView* shadowSRV;
	ID3D11SamplerState* shadowSampler;
	ID3D11RasterizerState* shadowRasterizer;
	SimpleVertexShader* shadowVS;
	DirectX::XMFLOAT4X4 shadowViewMatrix;
	DirectX::XMFLOAT4X4 shadowProjectionMatrix;
};


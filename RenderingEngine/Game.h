#pragma once

#define MAX_RIPPLES 32
#define RIPPLE_DURATION 10

#include "Canvas.h"
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
#include "TreeManager.h"
#include "FishController.h"
#include "Emitter.h"
#include "VirtualVertices.h"
#include "AudioEngine.h"
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
	int numWaves = 5;
	void LoadShaders(); 
	void CreateCamera();
	void InitializeEntities();
	void InitializeRenderer();
	void DrawSky();

	Entity* refractionEntity;

	void DrawRefraction();
	void DrawFullscreenQuad(ID3D11ShaderResourceView* texture);
	void DrawPostProcess(ID3D11ShaderResourceView* texture);
	void BloomPostProcess(ID3D11ShaderResourceView* texture);
	void DepthOfFieldPostProcess(ID3D11ShaderResourceView*  texture);
	void LensFlare(ID3D11ShaderResourceView*  texture);

	void CreateRipple(float x, float y, float z, float duration, float ringSize = 0);
	bool projectileHitWater;
	bool isDofEnabled;
	std::vector<Ripple> ripples;

	SimpleVertexShader*			vertexShader;
	SimplePixelShader*			pixelShader;
	Camera*		camera;
	std::unique_ptr<Terrain> terrain;
	std::unique_ptr<Terrain> terrain2;

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

	//Post Processing
	ID3D11ShaderResourceView* postProcessSRV;
	ID3D11RenderTargetView* postProcessRTV;
	ID3D11ShaderResourceView* bloomExtractSRV;
	ID3D11RenderTargetView* bloomExtractRTV;
	ID3D11ShaderResourceView* bloomBlurSRV;
	ID3D11RenderTargetView* bloomBlurRTV;
	ID3D11ShaderResourceView* bloomSRV;
	ID3D11RenderTargetView* bloomRTV;

	ID3D11ShaderResourceView* dofBlurSRV;
	ID3D11RenderTargetView* dofBlurRTV;
	ID3D11ShaderResourceView* dofSRV;
	ID3D11RenderTargetView* dofRTV;

	ID3D11ShaderResourceView* lensFlareThresholdSRV;
	ID3D11RenderTargetView* lensFlareThresholdRTV;

	ID3D11ShaderResourceView* lensFlareSRV;
	ID3D11RenderTargetView* lensFlareRTV;

	ID3D11ShaderResourceView* ghostGenerateSRV;
	ID3D11RenderTargetView* ghostGenerateRTV;


	// An SRV is good enough for loading textures with the DirectX Toolkit and then
	// using them with shaders 
	ID3D11ShaderResourceView* textureSRV;
	ID3D11ShaderResourceView* normalMapSRV;
	bool prevEsc;
	float time, translate;
	float transparency = 0.45f;
	Water * water;
	SimpleDomainShader *domainShader;
	SimpleHullShader *hullShader;
	void CreateWater();
	void DrawWater();
	void Tesellation();
	void SetupPostProcess(bool resize = false);

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

	std::unique_ptr<TreeManager> trees;
	std::unique_ptr<FishController> fishes;
	Emitter* emitter;
	ID3D11BlendState* particleBlendState;
	ID3D11DepthStencilState* particleDepthState;
	std::vector<std::shared_ptr<Emitter>> emitters;

	//Virtual vertices for approximating collision on the water
	VirtualVertices virtualVertices;

	//Canvas
	Canvas *canvas;
	bool gameStarted;
	void Blur(ID3D11ShaderResourceView* texture);
	std::vector<ID3D11ShaderResourceView*> skyTextures;
	int currentSky = 0;
	bool prevSpaceBar;
};


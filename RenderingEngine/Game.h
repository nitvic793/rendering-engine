#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Entity.h"
#include "Material.h"
#include "Camera.h"
#include "Lights.h"
#include "WICTextureLoader.h"
#include "Renderer.h"

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
	void CreateQuad();

	ID3D11ShaderResourceView*	metalSRV;
	ID3D11ShaderResourceView*	metalNormalSRV;
	ID3D11ShaderResourceView*	fabricSRV;
	ID3D11ShaderResourceView*	fabricNormalSRV;
	ID3D11ShaderResourceView*	woodSRV;
	ID3D11ShaderResourceView*	woodNormalSRV;
	ID3D11SamplerState*			sampler;
	SimpleVertexShader*			vertexShader;
	SimplePixelShader*			pixelShader;

	Material*	material;
	Material*	fabricMaterial;
	Material*	woodMaterial;
	Camera*		camera;

	POINT prevMousePos;
	Renderer *renderer;

	DirectionalLight light;
	DirectionalLight secondaryLight;
	PointLight pointLight;
	std::unordered_map<std::string, DirectionalLight> lights;
	std::unordered_map<std::string, Light*> lightsMap;
	std::unordered_map<std::string, Mesh*> models;
	std::vector<Entity*> entities;	
};


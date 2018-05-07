#pragma once

#include "Button.h"
#include "CommonStates.h"
#include <d3d11.h>
#include <functional>
#include <iostream>
#include "Resources.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"

using namespace DirectX;
using namespace SimpleMath;
using namespace std;

class Canvas {
public:
	Canvas(ID3D11Device* device, ID3D11DeviceContext* context, Resources *resources);
	~Canvas();
	void OnMouseDown(int x, int y);
	void OnMouseUp(int x, int y);
	void OnMouseMove(int x, int y);
	void Update(float deltaTime);
	void Draw();
	void AssignMenuButtonFunction(function<void()> func);
	void AssignQuitButtonFunction(function<void()> func);
	void StartGame();
private:
	ID3D11Device * device;
	ID3D11DeviceContext* context;

	unique_ptr<SpriteBatch> spriteBatch;
	unique_ptr<SpriteFont> spriteFont;

	Button *menuButton;
	Button *quitButton;
};
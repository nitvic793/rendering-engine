#pragma once

#include <d3d11.h>
#include <functional>
#include <iostream>
#include "SimpleMath.h"
#include <string>

using namespace DirectX::SimpleMath;
using namespace std;

enum ButtonState {
	Normal = 0,
	Hover,
	Press
};

class Button {
public:
	Button(string text, int x, int y, int width, int height, ID3D11ShaderResourceView *srvNormal, ID3D11ShaderResourceView *srvHover, ID3D11ShaderResourceView *srvPress);
	~Button();
	void OnMouseDown(int x, int y);
	void OnMouseUp(int x, int y);
	void OnMouseMove(int x, int y);
	void Update(float deltaTime);
	ID3D11ShaderResourceView *GetSRV();
	Vector2 GetPosition();
	void AssignFunction(function<void()> func);
	void SetEnabled(bool enabled);
	bool IsEnabled();
	wstring GetText();
private:
	bool enabled;
	function<void()> func;
	string text;
	wstring wtext;
	int x;
	int y;
	int width;
	int height;
	ID3D11ShaderResourceView *srvNormal;
	ID3D11ShaderResourceView *srvHover;
	ID3D11ShaderResourceView *srvPress;
	ID3D11ShaderResourceView *activeSRV;

	ButtonState state;
};
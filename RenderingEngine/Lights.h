#pragma once

#include <DirectXMath.h>

#define MAX_LIGHTS 32

using namespace DirectX;

enum LightType { Directional, Point };

class ILight {};

struct DirectionalLight : public ILight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 Direction;
	float Padding;
};

struct PointLight : public ILight
{
	XMFLOAT4 Color;
	XMFLOAT3 Position;
	float Range;
};


struct Light
{
	ILight *LightInstance;
	LightType Type;

	template<typename T>
	T* GetLight()
	{
		return static_cast<T*>(LightInstance);
	}
};
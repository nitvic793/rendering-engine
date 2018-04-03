#pragma once
#include <DirectXMath.h>
const float RIPPLE_EXPAND_SPEED = 6.0f;

class Ripple {
public:
	Ripple(float x, float y, float z, float duration, float ringSize);
	~Ripple();
	void Update(float deltaTime);
	DirectX::XMFLOAT3 GetPosition();
	float GetRadius();
	float GetRingSize();
	bool AtMaxDuration();
private:
	float x;
	float y;
	float z;
	float radius;
	float ringSize;
	float lifeTime;
	float currentTime;
};
#include "Ripple.h"

Ripple::Ripple(float x, float y, float z, float duration, float ringSize) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->lifeTime = duration;
	this->radius = 0.0f;
	this->ringSize = ringSize;
	this->currentTime = 0.0f;
	this->intensity = 1.0f;
}

Ripple::~Ripple() {

}

void Ripple::Update(float deltaTime) {
	radius += RIPPLE_EXPAND_SPEED * deltaTime;
	currentTime += deltaTime;
	intensity = 1.0f - (currentTime / lifeTime);
}

DirectX::XMFLOAT3 Ripple::GetPosition() {
	return DirectX::XMFLOAT3(x, y, z);
}

float Ripple::GetRadius() {
	return radius;
}

float Ripple::GetRingSize() {
	return ringSize;
}

float Ripple::GetIntensity() {
	return intensity;
}

bool Ripple::AtMaxDuration() {
	return (currentTime >= lifeTime);
}

//Convert the class into a struct
RippleData Ripple::GetRippleData() {
	DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(x, y, z);
	RippleData r = RippleData{ pos, radius, ringSize, intensity };
	return r;
}
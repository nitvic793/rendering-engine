#include "Ripple.h"

Ripple::Ripple(float x, float y, float z, float duration, float ringSize) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->lifeTime = duration;
	this->radius = 0.0f;
	this->ringSize = ringSize;
	this->currentTime = 0.0f;
}

Ripple::~Ripple() {

}

void Ripple::Update(float deltaTime) {
	radius += RIPPLE_EXPAND_SPEED * deltaTime;
	currentTime += deltaTime;
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

bool Ripple::AtMaxDuration() {
	return (currentTime >= lifeTime);
}
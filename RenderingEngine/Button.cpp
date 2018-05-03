#include "Button.h"

Button::Button(string text, int x, int y, int width, int height, ID3D11ShaderResourceView *srvNormal, ID3D11ShaderResourceView *srvHover, ID3D11ShaderResourceView *srvPress) {
	this->text = text;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->srvNormal = srvNormal;
	this->srvHover = srvHover;
	this->srvPress = srvPress;

	this->activeSRV = srvNormal;
	this->enabled = true;
	state = ButtonState::Normal;
	this->wtext = wstring(text.begin(), text.end());
}

Button::~Button() {
}

void Button::OnMouseDown(int x, int y) {
	if (x >= this->x && x <= this->x + this->width &&
		y >= this->y && y <= this->y + this->height) {
		state = ButtonState::Press;
	}
}
void Button::OnMouseUp(int x, int y) {
	if (x >= this->x && x <= this->x + this->width &&
		y >= this->y && y <= this->y + this->height) {
		if (state == ButtonState::Press && func != nullptr) {
			func();
		}
		state = ButtonState::Hover;
	}
	else {
		state = ButtonState::Normal;
	}
}
void Button::OnMouseMove(int x, int y) {
	if (state == ButtonState::Press) return;
	if (x >= this->x && x <= this->x + this->width &&
		y >= this->y && y <= this->y + this->height) {
		state = ButtonState::Hover;
	}
	else {
		state = ButtonState::Normal;
	}

}

void Button::Update(float deltaTime) {
	switch (state) {
	case ButtonState::Normal:
		activeSRV = srvNormal;
		break;

	case ButtonState::Hover:
		activeSRV = srvHover;
		break;

	case ButtonState::Press:
		activeSRV = srvPress;
		break;
	}
}

ID3D11ShaderResourceView *Button::GetSRV() {
	return activeSRV;
}

Vector2 Button::GetPosition() {
	return Vector2(x, y);
}

void Button::AssignFunction(function<void()> func) {
	this->func = func;
}

void Button::SetEnabled(bool enabled) {
	this->enabled = enabled;
}

bool Button::IsEnabled() {
	return enabled;
}

wstring Button::GetText() {
	return wtext;
}
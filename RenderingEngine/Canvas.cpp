#include "Canvas.h"

Canvas::Canvas(ID3D11Device* device, ID3D11DeviceContext* context, Resources *resources) {
	this->device = device;
	this->context = context;
	res = resources;
	spriteBatch.reset(new SpriteBatch(context));
	spriteFont.reset(new SpriteFont(device, L"../../Assets/Fonts/Calibri.spritefont"));

	menuButton = new Button("", 599, 296, 190, 45, resources->GetSRV("button_normal"), resources->GetSRV("button_hover"), resources->GetSRV("button_press"));
	quitButton = new Button("", 1150, 600, 128, 128, resources->GetSRV("quit"), resources->GetSRV("quit_hover"), resources->GetSRV("quit"));
	quitButton->SetEnabled(false);
}

Canvas::~Canvas() {
	delete menuButton;
	delete quitButton;
}

void Canvas::OnMouseDown(int x, int y) {
	if (menuButton->IsEnabled())
		menuButton->OnMouseDown(x, y);

	if (quitButton->IsEnabled())
		quitButton->OnMouseDown(x, y);
}

void Canvas::OnMouseUp(int x, int y) {
	if (menuButton->IsEnabled())
		menuButton->OnMouseUp(x, y);

	if (quitButton->IsEnabled())
		quitButton->OnMouseUp(x, y);
}

void Canvas::OnMouseMove(int x, int y) {
	if (menuButton->IsEnabled())
		menuButton->OnMouseMove(x, y);

	if (quitButton->IsEnabled())
		quitButton->OnMouseMove(x, y);
}

void Canvas::Update(float deltaTime) {
	if (menuButton->IsEnabled())
		menuButton->Update(deltaTime);

	if (quitButton->IsEnabled())
		quitButton->Update(deltaTime);
}

void Canvas::Draw() {
	CommonStates states(device);
	spriteBatch->Begin(SpriteSortMode_Deferred, states.NonPremultiplied());
	
	if (menuButton->IsEnabled()) {
		spriteBatch->Draw(res->GetSRV("title"), Vector2(378,100));
		spriteBatch->Draw(menuButton->GetSRV(), menuButton->GetPosition());
		spriteFont->DrawString(spriteBatch.get(), menuButton->GetText().c_str(), menuButton->GetPosition() + Vector2(0, 50), Colors::Black);
	}

	if (quitButton->IsEnabled()) {
		spriteBatch->Draw(quitButton->GetSRV(), quitButton->GetPosition());
		spriteFont->DrawString(spriteBatch.get(), quitButton->GetText().c_str(), quitButton->GetPosition() + Vector2(20, 50), Colors::Black);
	}

	spriteBatch->End();
	context->OMSetBlendState(0, 0, 0xFFFFFFFF);
	context->OMSetDepthStencilState(0, 0);
}

void Canvas::AssignMenuButtonFunction(function<void()> func) {
	menuButton->AssignFunction(func);
}

void Canvas::AssignQuitButtonFunction(function<void()> func) {
	quitButton->AssignFunction(func);
}

void Canvas::StartGame() {
	menuButton->SetEnabled(false);
	quitButton->SetEnabled(false);
}
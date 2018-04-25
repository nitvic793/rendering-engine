#pragma once
#include "Renderer.h"
#include "Entity.h"

class FishController
{
	int fishCount;
	XMFLOAT3 startPosition;
	XMFLOAT3 endPosition;
	float resetThresholdDistance;
	std::vector<Entity*> entities;
	XMFLOAT3 RandomOffsetFromStart();
	XMFLOAT3 rotation;
	float speed;
public:
	void Update(float deltaTime, float totalTime);
	void Render(Renderer* renderer);
	bool CheckForCollision(Entity* entity);
	FishController(Mesh* mesh, Material* mat, int count, XMFLOAT3 startPos, XMFLOAT3 endPos, float resetThreshold, XMFLOAT3 defaultRotation, XMFLOAT3 defaultScale);
	~FishController();
};


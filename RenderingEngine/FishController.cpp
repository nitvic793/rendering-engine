#include "FishController.h"
#include <time.h>


XMFLOAT3 FishController::RandomOffsetFromStart()
{
	float xOffset = (float)(rand() % 1200 + 1) / 50;
	float zOffset = -(float)(rand() % 1800 + 1) / 50;
	auto pos = startPosition;
	pos.x += xOffset;
	pos.z += zOffset;
	return pos;
}

void FishController::Update(float deltaTime, float totalTime)
{
	for (auto e : entities)
	{
		auto pos = e->GetPosition();
		if (pos.z >= endPosition.z)
		{
			e->SetPosition(RandomOffsetFromStart());
		}

		e->Move(XMFLOAT3((sin(totalTime * 3) / 600), 0, speed*deltaTime));
	}
}

void FishController::Render(Renderer* renderer)
{
	for (auto e : entities)
	{
		renderer->Draw(e);
	}
}

bool FishController::CheckForCollision(Entity * entity)
{
	for (auto e : entities)
	{
		if (entity->GetBoundingBox().Intersects(e->GetBoundingBox()))
		{
			e->SetPosition(RandomOffsetFromStart());
			return true;
		}
	}

	return false;
}

FishController::FishController(Mesh* mesh, Material* mat, int count, XMFLOAT3 startPos, XMFLOAT3 endPos, float resetThreshold, XMFLOAT3 defaultRotation, XMFLOAT3 defaultScale)
{
	speed = 4.f;
	fishCount = count;
	startPosition = startPos;
	endPosition = endPos;
	rotation = defaultRotation;
	resetThresholdDistance = resetThreshold;
	for (int i = 0; i < count; ++i)
	{
		Entity *entity = new Entity(mesh, mat);
		entity->isAnimated = true;	
		entity->SetPosition(RandomOffsetFromStart());
		entity->SetRotation(rotation.x, rotation.y, rotation.z);

		//if (i % 2 == 0)
			entity->SetScale(defaultScale.x, defaultScale.y, defaultScale.z);
		//else
			//entity->SetScale(defaultScale.x, defaultScale.y, -defaultScale.z);

		entities.push_back(entity);
	}

	srand((unsigned int)time(nullptr));
	
}


FishController::~FishController()
{
	for (auto entity : entities)
	{
		delete entity;
	}
	entities.clear();
}

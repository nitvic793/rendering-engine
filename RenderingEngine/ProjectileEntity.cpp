#include "ProjectileEntity.h"



void ProjectileEntity::SetHasBeenShot(bool shot)
{
	hasBeenShot = false;
}

void ProjectileEntity::Update(float deltaTime, float totalTime)
{
	if (hasBeenShot)
	{
		auto pos = XMLoadFloat3(&position);
		auto dir = XMLoadFloat3(&shotDirection);
		pos = pos + dir * speed;
		XMStoreFloat3(&position, pos);
	}
	else
	{
		auto pos = XMLoadFloat3(&position);
		auto dir = -XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)));
		XMVECTOR up = XMVectorSet(0, 1, 0, 0); // Y is up!
		float speed = 10.f;
		if (GetAsyncKeyState('W') & 0x8000)
		{
			pos = pos + dir * speed * deltaTime;
		}

		if (GetAsyncKeyState('S') & 0x8000)
		{
			pos = pos - dir * speed * deltaTime;
		}

		if (GetAsyncKeyState('A') & 0x8000)
		{
			auto leftDir = XMVector3Cross(dir, up);
			pos = pos + leftDir * speed * deltaTime;;
		}

		if (GetAsyncKeyState('D') & 0x8000)
		{
			auto rightDir = XMVector3Cross(-dir, up);
			pos = pos + rightDir * speed * deltaTime;;
		}

		XMStoreFloat3(&position, pos);
	}
}

void ProjectileEntity::Shoot(float speed, XMFLOAT3 direction)
{
	hasBeenShot = true;
	this->speed = speed;
	this->shotDirection = direction;
}

ProjectileEntity::ProjectileEntity(Mesh* mesh, Material *mat) :
	Entity(mesh, mat)
{
	hasBeenShot = false;
}


ProjectileEntity::~ProjectileEntity()
{
}

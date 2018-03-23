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

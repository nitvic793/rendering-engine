#pragma once

#include "Entity.h"
#include "Mesh.h"
#include "Material.h"

class ProjectileEntity : public Entity
{
protected:
	float speed;
	XMFLOAT3 shotDirection;
	bool hasBeenShot;
public:
	void SetHasBeenShot(bool shot);
	bool HasBeenShot();
	void Update(float deltaTime, float totalTime) override;
	void Shoot(float speed, XMFLOAT3 direction);
	ProjectileEntity(Mesh* mesh, Material *mat);
	~ProjectileEntity();
};


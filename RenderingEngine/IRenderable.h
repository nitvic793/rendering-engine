#pragma once
class IRenderable
{
public:
	virtual void Render() = 0;
	IRenderable();
	~IRenderable();
};


#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The position of the vertex
	DirectX::XMFLOAT3 Normal;        
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Tangent;
};


struct VertexAnimated
{
	DirectX::XMFLOAT4 Position{ 0,0,0,0 };
	DirectX::XMFLOAT3 Normal{ 0,0,0 };
	DirectX::XMFLOAT2 UV{0,0};
	DirectX::XMFLOAT3 Tangent{ 0,0 ,0};
	DirectX::XMFLOAT4 Boneids{0,0,0,0};
	DirectX::XMFLOAT4 Weights{ 0,0,0,0 };

struct VertexTerrain
{
	DirectX::XMFLOAT3 Position;	    // The position of the vertex
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT2 BlendUV;
};
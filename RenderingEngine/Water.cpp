#include "Water.h"
using namespace DirectX;


Water::Water()
{
}


Water::~Water()
{
}


const int u = 128;
const int v = 64;
const int IndicesCount = (u - 1)*(v - 1) * 6;
void Water::GenerateLandscape()
{
	int waterRadius = 10;
	Vertex vertices[6];
	// Load the vertex array with data.
	vertices[0].Position = XMFLOAT3(-waterRadius, 0.0f, waterRadius);  // Top left.

	vertices[1].Position = XMFLOAT3(waterRadius, 0.0f, waterRadius);  // Top right.

	vertices[2].Position = XMFLOAT3(-waterRadius, 0.0f, -waterRadius);  // Bottom left.

	vertices[3].Position = XMFLOAT3(waterRadius, 0.0f, -waterRadius);  // Bottom right.

	int indices[6] = {0,1,2,2,1,4};

}

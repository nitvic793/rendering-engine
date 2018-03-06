#include "Water.h"
using namespace DirectX;


Water::Water(int _length,int _breadth)
{
	XMStoreFloat4x4(&reflectionmatrix, XMMatrixTranspose(XMMatrixIdentity()));
	reflectionBuffer = 0;
	length = _length;
	breadth = _breadth;
	vertices = new Vertex[length * breadth];
}


Water::~Water()
{
	delete indices;
	delete vertices;
}

// -----------------------------------------------------
// Generate the triangles/quad for the water surface
// -----------------------------------------------------
void Water::GenerateWaterMesh()
{
	const int x = length;
	const int y = breadth;
	const int IndicesCount = (x - 1)*(y - 1) * 6;
	indices = new UINT[IndicesCount];
	
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			//vertices[j*x + i].Position = XMFLOAT3(i, 0, j);
			vertices[j*x + i].Position.x = i;
			vertices[j*x + i].Position.y = 0;
			vertices[j*x + i].Position.z = j;
		}
	}

	for (int i = 0; i<(x - 1); i++)
		for (int j = 0; j<(y - 1); j++)
		{
			unsigned int indexa = j * (x - 1) + i;
			unsigned int indexb = j * y + i;
			indices[indexa * 6 + 0] = indexb;
			indices[indexa * 6 + 1] = indexb + 1 + x;
			indices[indexa * 6 + 2] = indexb + 1;

			indices[indexa * 6 + 3] = indexb;
			indices[indexa * 6 + 4] = indexb + x;
			indices[indexa * 6 + 5] = indexb + x + 1;
		}
}

UINT* Water::GetIndices()
{
	return indices;
}

Vertex* Water::GetVertices()
{
	return vertices;
}

UINT Water::GetVertexCount()
{
	return length * breadth;
}
UINT Water::GetIndexCount()
{
	return (length - 1) * (breadth - 1) * 6;
}
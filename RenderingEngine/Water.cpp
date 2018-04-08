#include "Water.h"
using namespace DirectX;


Water::Water(int _length,int _breadth)
{
	XMStoreFloat4x4(&reflectionmatrix, XMMatrixTranspose(XMMatrixIdentity()));
	reflectionBuffer = 0;
	length = _length;
	breadth = _breadth;
	vertices = new Vertex[length * breadth];
	indices = new UINT[length * breadth * 6];
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
	
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			// Vertex positions
			vertices[j*x + i].Position.x = (float)i;
			vertices[j*x + i].Position.y = 0;
			vertices[j*x + i].Position.z = (float)j;
			
			// Vertex normals
			vertices[j*x + i].Normal.x = 0;
			vertices[j*x + i].Normal.y = 1;
			vertices[j*x + i].Normal.z = 0;

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

void Water::CalculateUVCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;

	// Calculate how much to increment the texture coordinates by.
	// Increase water fidelity by increasing this value
	// Change numbers in powers of 2
	incrementValue = (float)16 / (float)breadth;
	
	// Calculate how many times to repeat the texture.
	incrementCount = breadth;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	for (j = 0; j<length; j++)
	{
		for (i = 0; i<breadth; i++)
		{
			vertices[(length * j) + i].UV.x = tuCoordinate; //u
			vertices[(length * j) + i].UV.y = tvCoordinate; //v

			// Increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			// Check if at the far right end of the texture and if so then start at the beginning again.
			if (tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if (tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

}

void Water::CreateWaves()
{
	for (int i = 0; i < NUM_OF_WAVES; i++)
	{
		float randomAngle = (float)(rand() * XM_PI );
		waves[i].direction.x = sin(randomAngle);
		waves[i].direction.y = sin(randomAngle);
		/*waves[i].amplitude = 0.03f + pow(2.0f, (float)rand() * 2.0f) * 0.05f;
		waves[i].wavelength = 1.0f + pow(2.f, 1.f + (float)rand()) * 10.f;*/
		waves[i].amplitude = 1;
		waves[i].wavelength = 4;
	}
}

Wave* Water::GetWaves()
{
	return waves;
}

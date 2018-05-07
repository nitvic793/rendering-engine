#include "Water.h"
using namespace DirectX;

// -----------------------------------------------------
// Initialize constructor
// -----------------------------------------------------
Water::Water(int _length,int _breadth) : Entity(nullptr,nullptr)
{
	XMStoreFloat4x4(&reflectionmatrix, XMMatrixTranspose(XMMatrixIdentity()));
	reflectionBuffer = 0;
	length = _length;
	breadth = _breadth;
	vertices = new Vertex[length * breadth];
	indices = new UINT[length * breadth * 6];
}

// -----------------------------------------------------
// Cleanup
// -----------------------------------------------------
Water::~Water()
{
	delete mesh;
	delete indices;
	delete vertices;
}

//--------------------------------------------------------
// Initialize water entity
//--------------------------------------------------------
void Water::Init(Material* mat, ID3D11Device * device)
{
	GenerateWaterMesh();
	CalculateUVCoordinates();
	mesh = new Mesh(vertices, this->GetVertexCount(), indices, this->GetIndexCount(), device);
	material = mat;
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

// -----------------------------------------------------
// Retrun indices
// -----------------------------------------------------
UINT* Water::GetIndices() const
{
	return indices;
}

// -----------------------------------------------------
// Return vertices
// -----------------------------------------------------
Vertex* Water::GetVertices() const
{
	return vertices;
}

// -----------------------------------------------------
// Return number of vertices
// -----------------------------------------------------
UINT Water::GetVertexCount() const
{
	return length * breadth;
}

// -----------------------------------------------------
// Return number of indices
// -----------------------------------------------------
UINT Water::GetIndexCount() const
{
	return (length - 1) * (breadth - 1) * 6;
}

// -----------------------------------------------------
// Calculate UVs fro water surface
// -----------------------------------------------------
void Water::CalculateUVCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;

	// Calculate how much to increment the texture coordinates by.
	// Increase water fidelity by increasing this value
	// Change numbers in powers of 2
	incrementValue = (float)4 / (float)breadth;
	
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

// --------------------------------------------------------------
// Create the waves which are combined to form the final waveform
// --------------------------------------------------------------
void Water::CreateWaves()
{
	waves[0] = Wave{ XMFLOAT2(0,1),0.4f,6 };
	waves[1] = Wave{ XMFLOAT2(1,1),0.2f,10 };
	waves[2] = Wave{ XMFLOAT2(0,1),0.6f,20 };
	waves[3] = Wave{ XMFLOAT2(1,1),0.1f,3 };
	waves[4] = Wave{ XMFLOAT2(1,0),0.2f,6 };
}

// -----------------------------------------------------
// Return the generated waves
// -----------------------------------------------------
Wave* Water::GetWaves()
{
	return waves;
}
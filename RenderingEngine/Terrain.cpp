#include "Terrain.h"

const int Terrain::GetTerrainHeight()
{
	return terrainHeight;
}

const int Terrain::GetTerrainWidth()
{
	return terrainWidth;
}

void Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	XMFLOAT3* normals;

	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new XMFLOAT3[(terrainHeight - 1) * (terrainWidth - 1)];
	heightNormals = new XMFLOAT3[terrainWidth * terrainHeight];
	if (!normals)
	{
		return;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j<(terrainHeight - 1); j++)
	{
		for (i = 0; i<(terrainWidth - 1); i++)
		{
			index1 = (j * terrainHeight) + i;
			index2 = (j * terrainHeight) + (i + 1);
			index3 = ((j + 1) * terrainHeight) + i;

			// Get three vertices from the face.
			vertex1[0] = heightMap[index1].x;
			vertex1[1] = heightMap[index1].y;
			vertex1[2] = heightMap[index1].z;

			vertex2[0] = heightMap[index2].x;
			vertex2[1] = heightMap[index2].y;
			vertex2[2] = heightMap[index2].z;

			vertex3[0] = heightMap[index3].x;
			vertex3[1] = heightMap[index3].y;
			vertex3[2] = heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (terrainHeight - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for (j = 0; j<terrainHeight; j++)
	{
		for (i = 0; i<terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if ((i < (terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (terrainHeight - 1)))
			{
				index = (j * (terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if ((i < (terrainWidth - 1)) && (j < (terrainHeight - 1)))
			{
				index = (j * (terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			heightNormals[index].x = (sum[0] / length);
			heightNormals[index].y = (sum[1] / length);
			heightNormals[index].z = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;
}

void Terrain::CalculateUVCoordinates()
{
	int incrementCount, i, j, tuCount, tvCount;
	float incrementValue, tuCoordinate, tvCoordinate;
	textureCoords = new XMFLOAT2[terrainHeight * terrainWidth];

	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)terrainWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = terrainWidth / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	for (j = 0; j<terrainHeight; j++)
	{
		for (i = 0; i<terrainWidth; i++)
		{
			// Store the texture coordinate in the height map.
			textureCoords[(terrainHeight * j) + i].x = tuCoordinate; //u
			textureCoords[(terrainHeight * j) + i].y = tvCoordinate; //v

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

void Terrain::SetTextures(DXTexPtr red, DXTexPtr green, DXTexPtr blue, DXTexPtr alpha)
{
	redTexture = red;
	blueTexture = blue;
	greenTexture = green;
	alphaTexture = alpha;
}

void Terrain::Initialize(ID3D11Device * device, ID3D11DeviceContext * context)
{
	mesh = new Mesh();
	int vertexCount = (terrainWidth - 1) * (terrainHeight - 1) * 8;
	int indexCount = vertexCount;
	int index = 0;
	float positionX, positionZ;
	Vertex *vertices = new Vertex[vertexCount];
	UINT *indices = new UINT[indexCount];

	for (int j = 0; j<(terrainHeight - 1); j++)
	{
		for (int i = 0; i<(terrainWidth - 1); i++)
		{
			// LINE 1
			// Upper left.
			positionX = (float)i;
			positionZ = (float)(j + 1);

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Upper right.
			positionX = (float)(i + 1);
			positionZ = (float)(j + 1);

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// LINE 2
			// Upper right.
			positionX = (float)(i + 1);
			positionZ = (float)(j + 1);

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Bottom right.
			positionX = (float)(i + 1);
			positionZ = (float)j;

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// LINE 3
			// Bottom right.
			positionX = (float)(i + 1);
			positionZ = (float)j;

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Bottom left.
			positionX = (float)i;
			positionZ = (float)j;

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// LINE 4
			// Bottom left.
			positionX = (float)i;
			positionZ = (float)j;

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;

			// Upper left.
			positionX = (float)i;
			positionZ = (float)(j + 1);

			vertices[index].Position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[index].Normal = XMFLOAT3(0.f, 0.0f, -1.f);
			vertices[index].UV = XMFLOAT2(0.f, 0.0f);
			//vertices[index].color = XMFLOAT3(1.0f, 1.0f, 1.0f, 1.0f);
			indices[index] = index;
			index++;
		}
	}

	mesh->Initialize(vertices, vertexCount, indices, indexCount, device);
	delete vertices;
	delete indices;
}

bool Terrain::Initialize(const char* filename, ID3D11Device * device, ID3D11DeviceContext * context)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = (UINT)fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), (size_t)1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = (UINT)fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	terrainWidth = bitmapInfoHeader.biWidth;
	terrainHeight = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = terrainWidth * terrainHeight * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		return false;
	}

	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	count = (UINT)fread(bitmapImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	heightMap = new XMFLOAT3[terrainWidth * terrainHeight];
	if (!heightMap)
	{
		return false;
	}

	k = 0;

	for (j = 0; j<terrainHeight; j++)
	{
		for (i = 0; i<terrainWidth; i++)
		{
			height = bitmapImage[k];

			index = (terrainWidth * (terrainHeight - 1 - j)) + i;

			heightMap[index].x = (float)i;
			heightMap[index].y = (float)height;
			heightMap[index].z = -(float)j;
			heightMap[index].z += (float)(terrainHeight - 1);

			k += 3;
		}
	}

	//Remove spikes and smoothen terrain
	for (j = 0; j<terrainHeight; j++)
	{
		for (i = 0; i<terrainWidth; i++)
		{
			heightMap[(terrainHeight * j) + i].y /= 12.0f;
		}
	}

	delete[] bitmapImage;
	bitmapImage = 0;

	CalculateNormals();
	CalculateUVCoordinates();
	int vertexCount = (terrainWidth - 1) * (terrainHeight - 1) * 6;
	indexCount = vertexCount;
	index = 0;
	vertices = new VertexTerrain[vertexCount];
	indices = new UINT[indexCount];
	float tu, tv;

	for (int j = 0; j<(terrainHeight - 1); j++)
	{
		for (int i = 0; i<(terrainWidth - 1); i++)
		{
			int index1 = (terrainWidth * j) + i;          // Upper left.
			int index2 = (terrainWidth * j) + (i + 1);      // Upper right.
			int index3 = (terrainWidth * (j + 1)) + i;      // Bottom left.
			int index4 = (terrainWidth * (j + 1)) + (i + 1);  // Bottom right.


															 // Upper left.
			tv = textureCoords[index3].y;

			// Modify the texture coordinates to cover the top edge.
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].Position = XMFLOAT3(heightMap[index3].x, heightMap[index3].y, heightMap[index3].z);
			vertices[index].UV = XMFLOAT2(0, 0);
			vertices[index].BlendUV = XMFLOAT2(textureCoords[index3].x, tv);
			vertices[index].Normal = XMFLOAT3(heightNormals[index3].x, heightNormals[index3].y, heightNormals[index3].z);
			indices[index] = index;
			index++;

			// Upper right.
			tu = textureCoords[index4].x;
			tv = textureCoords[index4].y;

			// Modify the texture coordinates to cover the top and right edge.
			if (tu == 0.0f) { tu = 1.0f; }
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].Position = XMFLOAT3(heightMap[index4].x, heightMap[index4].y, heightMap[index4].z);
			vertices[index].UV = XMFLOAT2(1, 0);
			vertices[index].BlendUV = XMFLOAT2(tu, tv);
			vertices[index].Normal = XMFLOAT3(heightNormals[index4].x, heightNormals[index4].y, heightNormals[index4].z);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].Position = XMFLOAT3(heightMap[index1].x, heightMap[index1].y, heightMap[index1].z);
			vertices[index].UV = XMFLOAT2(0,1);
			vertices[index].BlendUV = XMFLOAT2(textureCoords[index1].x, textureCoords[index1].y);
			vertices[index].Normal = XMFLOAT3(heightNormals[index1].x, heightNormals[index1].y, heightNormals[index1].z);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].Position = XMFLOAT3(heightMap[index1].x, heightMap[index1].y, heightMap[index1].z);
			vertices[index].UV = XMFLOAT2(0, 1);
			vertices[index].BlendUV = XMFLOAT2(textureCoords[index1].x, textureCoords[index1].y);
			vertices[index].Normal = XMFLOAT3(heightNormals[index1].x, heightNormals[index1].y, heightNormals[index1].z);
			indices[index] = index;
			index++;

			// Upper right.
			tu = textureCoords[index4].x;
			tv = textureCoords[index4].y;

			// Modify the texture coordinates to cover the top and right edge.
			if (tu == 0.0f) { tu = 1.0f; }
			if (tv == 1.0f) { tv = 0.0f; }

			vertices[index].Position = XMFLOAT3(heightMap[index4].x, heightMap[index4].y, heightMap[index4].z);
			vertices[index].UV = XMFLOAT2(1, 0);
			vertices[index].BlendUV = XMFLOAT2(tu, tv);
			vertices[index].Normal = XMFLOAT3(heightNormals[index4].x, heightNormals[index4].y, heightNormals[index4].z);
			indices[index] = index;
			index++;

			// Bottom right.
			tu = textureCoords[index2].x;

			// Modify the texture coordinates to cover the right edge.
			if (tu == 0.0f) { tu = 1.0f; }

			vertices[index].Position = XMFLOAT3(heightMap[index2].x, heightMap[index2].y, heightMap[index2].z);
			vertices[index].UV = XMFLOAT2(1, 1);
			vertices[index].BlendUV = XMFLOAT2(tu, textureCoords[index2].y);
			vertices[index].Normal = XMFLOAT3(heightNormals[index2].x, heightNormals[index2].y, heightNormals[index2].z);
			indices[index] = index;
			index++;
		}
	}
	mesh = new Mesh();
	mesh->Initialize(vertices, vertexCount, indices, indexCount, device);
	delete vertices;
	delete indices;
	return true;
}

void Terrain::SetSplatMap(ID3D11ShaderResourceView * splat)
{
	splatMap = splat;
}

void Terrain::PrepareMaterialWithShadows(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT4X4 shadowViewMatrix, XMFLOAT4X4 shadowProjectionMatrix, ID3D11SamplerState * shadowSampler, ID3D11ShaderResourceView * shadowSRV)
{
	auto vertexShader = material->GetVertexShader();
	auto pixelShader = material->GetPixelShader();
	vertexShader->SetMatrix4x4("world", GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);
	vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
	vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);
	pixelShader->SetSamplerState("basicSampler", material->GetSampler());
	pixelShader->SetSamplerState("shadowSampler", shadowSampler);
	pixelShader->SetShaderResourceView("diffuseTexture", greenTexture);
	pixelShader->SetShaderResourceView("shadowMapTexture", shadowSRV);
	pixelShader->SetShaderResourceView("redTexture", redTexture);
	pixelShader->SetShaderResourceView("blueTexture", blueTexture);
	pixelShader->SetShaderResourceView("alphaTexture", alphaTexture);
	pixelShader->SetShaderResourceView("splatMap", splatMap);
	pixelShader->SetShaderResourceView("normalTexture", material->GetNormalSRV());
	pixelShader->SetShaderResourceView("roughnessTexture", material->GetRoughnessSRV());
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();
}

void Terrain::PrepareMaterial(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix)
{
	auto vertexShader = material->GetVertexShader();
	auto pixelShader = material->GetPixelShader();
	vertexShader->SetMatrix4x4("world", GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);

	pixelShader->SetSamplerState("basicSampler", material->GetSampler());
	pixelShader->SetShaderResourceView("diffuseTexture", redTexture);
	if (material->GetNormalSRV())
		pixelShader->SetShaderResourceView("normalTexture", material->GetNormalSRV());
	else
		pixelShader->SetShaderResourceView("normalTexture", nullptr);

	pixelShader->SetShaderResourceView("roughnessTexture", material->GetRoughnessSRV());
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();
}

Terrain::Terrain():
	Entity(nullptr,nullptr)
{
	terrainHeight = terrainWidth = 100;
	position = XMFLOAT3(0, 0, 0);
}


Terrain::~Terrain()
{
	delete mesh;
	delete heightMap;
	delete heightNormals;
	delete textureCoords;
}

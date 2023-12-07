/**
* Steps to draw the chunks and
* check and update neighbour chunks if needed.
*
* 1. Generate blocks.
* 2. Init mesh data
* 3. Load mesh data.
*/

#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>
#include <array>

#include "../../engine/renderer/mesh.h"
#include "../../engine/ray/ray.h"

#include "block.h"
#include "chunk.h"

using namespace Engine::Renderer;

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr uint32_t g_nBlocks = g_chunkSize.x * g_chunkSize.y * g_chunkSize.z;
constexpr uint32_t g_vertexPerCube = 36;

constexpr float g_rayDeltaMag = 0.1f;

using VertexArray = std::array<Vertex, g_vertexPerFace>;

constexpr VertexArray back{ {
	{{ 0, 0, 0 }, 0, 0 },
	{{ 0, 1, 0 }, 2, 0 },
	{{ 1, 0, 0 }, 1, 0 },

	{{ 1, 0, 0 }, 1, 0 },
	{{ 0, 1, 0 }, 2, 0 },
	{{ 1, 1, 0 }, 3, 0 },
} };

constexpr VertexArray front{ {
	{{ 0, 0, 1 }, 0, 0 },
	{{ 1, 0, 1 }, 1, 0 },
	{{ 0, 1, 1 }, 2, 0 },

	{{ 1, 0, 1 }, 1, 0 },
	{{ 1, 1, 1 }, 3, 0 },
	{{ 0, 1, 1 }, 2, 0 },
} };

constexpr VertexArray top{ {
	{{ 0, 1, 1 }, 0, 0 },
	{{ 1, 1, 1 }, 1, 0 },
	{{ 0, 1, 0 }, 2, 0 },

	{{ 1, 1, 1 }, 1, 0 },
	{{ 1, 1, 0 }, 3, 0 },
	{{ 0, 1, 0 }, 2, 0 },
} };

constexpr VertexArray bottom{ {
	{{ 0, 0, 1 }, 0, 0 },
	{{ 0, 0, 0 }, 2, 0 },
	{{ 1, 0, 1 }, 1, 0 },

	{{ 1, 0, 0 }, 3, 0 },
	{{ 1, 0, 1 }, 1, 0 },
	{{ 0, 0, 0 }, 2, 0 },
} };

constexpr VertexArray left{ {
	{{ 0, 0, 0 }, 0, 0 },
	{{ 0, 0, 1 }, 1, 0 },
	{{ 0, 1, 0 }, 2, 0 },
						   
	{{ 0, 0, 1 }, 1, 0 },
	{{ 0, 1, 1 }, 3, 0 },
	{{ 0, 1, 0 }, 2, 0 },
} };

constexpr VertexArray right{ {
	{{ 1, 0, 1 }, 0, 0 },
	{{ 1, 0, 0 }, 1, 0 },
	{{ 1, 1, 1 }, 2, 0 },

	{{ 1, 0, 0 }, 1, 0 },
	{{ 1, 1, 0 }, 3, 0 },
	{{ 1, 1, 1 }, 2, 0 }
} };

using FaceMap = std::unordered_map<Face::FaceType, const VertexArray, EnumHash>;
FaceMap g_faces = {
	{Face::FaceType::TOP,		top},
	{Face::FaceType::BOTTOM,	bottom},
	{Face::FaceType::FRONT,		front},
	{Face::FaceType::BACK,		back},
	{Face::FaceType::RIGHT,		right},
	{Face::FaceType::LEFT,		left},
};

namespace Noise
{
	static constexpr uint32_t p[512] = {
	   151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
	   8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203,
	   117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74,
	   165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220,
	   105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132,
	   187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186,
	   3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59,
	   227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70,
	   221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178,
	   185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
	   81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176,
	   115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195,
	   78, 66, 215, 61, 156, 180,
	};

	float perlin2D(float x, float y)
	{
		auto lerp = [](float a0, float a1, float t) {
			return (a1 - a0) * t + a0;
		};

		auto fade = [](float t) {
			return ((6 * t - 15) * t + 10) * t * t * t;
		};

		auto dot_grad = [](int hash, const float x, const float y) {
			// In 2D case, the gradient may be any of 8 direction vectors pointing to the
			// edges of a unit-square. The distance vector is the input offset (relative to
			// the smallest bound).
			switch (hash & 0x7)
			{
			case 0x0: return  x + y;
			case 0x1: return  x;
			case 0x2: return  x - y;
			case 0x3: return -y;
			case 0x4: return -x - y;
			case 0x5: return -x;
			case 0x6: return -x + y;
			case 0x7: return  y;
			default:  return  0.0f;
			}
		};

		// Top-left coordinates of the unit-square.
		const int xi0 = static_cast<int>(std::floor(x));
		const int yi0 = static_cast<int>(std::floor(y));

		// Input location in the unit-square.
		const float xf0 = x - xi0;
		const float yf0 = y - yi0;
		const float xf1 = xf0 - 1.0f;
		const float yf1 = yf0 - 1.0f;

		// Wrap to range 0-255.
		const int xi = xi0 & 0xFF;
		const int yi = yi0 & 0xFF;

		// Apply the fade function to the location.
		const float u = fade(xf0);
		const float v = fade(yf0);

		// Generate hash values for each point of the unit-square.
		const int h00 = p[p[xi + 0] + yi + 0];
		const int h01 = p[p[xi + 0] + yi + 1];
		const int h10 = p[p[xi + 1] + yi + 0];
		const int h11 = p[p[xi + 1] + yi + 1];

		// Linearly interpolate between dot products of each gradient with its distance to the input location.
		const float v1 = lerp(dot_grad(h00, xf0, yf0), dot_grad(h10, xf1, yf0), u);
		const float v2 = lerp(dot_grad(h01, xf0, yf1), dot_grad(h11, xf1, yf1), u);

		return lerp(v1, v2, v);
	}

	float perlin3D(float x, float y, float z)
	{
		auto lerp = [](float a0, float a1, float w) {
			return (a1 - a0) * w + a0;
		};

		auto fade = [](float t) {
			return ((6 * t - 15) * t + 10) * t * t * t;
		};

		auto dot_grad = [](int hash, float x, float y, float z) {
			switch (hash & 0xF)
			{
			case 0x0: return  x + y;
			case 0x1: return -x + y;
			case 0x2: return  x - y;
			case 0x3: return -x - y;
			case 0x4: return  x + z;
			case 0x5: return -x + z;
			case 0x6: return  x - z;
			case 0x7: return -x - z;
			case 0x8: return  y + z;
			case 0x9: return -y + z;
			case 0xA: return  y - z;
			case 0xB: return -y - z;
			case 0xC: return  y + x;
			case 0xD: return -y + z;
			case 0xE: return  y - x;
			case 0xF: return -y - z;
			default: return 0.0f; // never happens I hope
			}
		};

		// Bottom-left coordinates of the unit-square.
		const int xi0 = static_cast<int>(std::floor(x));
		const int yi0 = static_cast<int>(std::floor(y));
		const int zi0 = static_cast<int>(std::floor(z));

		// Input location in the unit-square.
		const float xf0 = x - xi0;
		const float yf0 = y - yi0;
		const float zf0 = z - zi0;
		const float xf1 = xf0 - 1.0f;
		const float yf1 = yf0 - 1.0f;
		const float zf1 = zf0 - 1.0f;

		// Wrap to range 0-255.
		const int xi = xi0 & 0xFF;
		const int yi = yi0 & 0xFF;
		const int zi = zi0 & 0xFF;

		// Apply the fade function to the location.
		const float u = fade(xf0);
		const float v = fade(yf0);
		const float w = fade(zf0);

		// Generate hash values for each point of the unit-square.
		const int h000 = p[p[p[xi + 0] + yi + 0]	+ zi + 0];
		const int h001 = p[p[p[xi + 0] + yi + 0]	+ zi + 1];
		const int h010 = p[p[p[xi + 0] + yi + 1]	+ zi + 0];
		const int h011 = p[p[p[xi + 0] + yi + 1]	+ zi + 1];
		const int h100 = p[p[p[xi + 1] + yi + 0]	+ zi + 0];
		const int h101 = p[p[p[xi + 1] + yi + 0]	+ zi + 1];
		const int h110 = p[p[p[xi + 1] + yi + 1]	+ zi + 0];
		const int h111 = p[p[p[xi + 1] + yi + 1]	+ zi + 1];

		float x1, x2, y1, y2;

		x1 = lerp(dot_grad(h000, xf0, yf0, zf0), dot_grad(h100, xf1, yf0, zf0), u);
		x2 = lerp(dot_grad(h010, xf0, yf1, zf0), dot_grad(h110, xf1, yf1, zf0), u);
		y1 = lerp(x1, x2, v);

		x1 = lerp(dot_grad(h001, xf0, yf0, zf1), dot_grad(h101, xf1, yf0, zf1), u);
		x2 = lerp(dot_grad(h011, xf0, yf1, zf1), dot_grad(h111, xf1, yf1, zf1), u);
		y2 = lerp(x1, x2, v);

		return lerp(y1, y2, w);
	}

	float simplex3D(float x, float y, float z) 
	{
		float n0, n1, n2, n3; // Noise contributions from the four corners

		// Skewing/Unskewing factors for 3D
		static const float F3 = 1.0f / 3.0f;
		static const float G3 = 1.0f / 6.0f;

		// Skew the input space to determine which simplex cell we're in
		float s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
		int i = std::floor(x + s);
		int j = std::floor(y + s);
		int k = std::floor(z + s);
		float t = (i + j + k) * G3;
		float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
		float Y0 = j - t;
		float Z0 = k - t;
		float x0 = x - X0; // The x,y,z distances from the cell origin
		float y0 = y - Y0;
		float z0 = z - Z0;

		// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
		// Determine which simplex we are in.
		int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
		int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
		if (x0 >= y0) 
		{
			if (y0 >= z0) 
			{
				i1 = 1; 
				j1 = 0; 
				k1 = 0;
				i2 = 1; 
				j2 = 1;
				k2 = 0; // X Y Z order
			}
			else if (x0 >= z0) 
			{
				i1 = 1;
				j1 = 0;
				k1 = 0; 
				i2 = 1; 
				j2 = 0;
				k2 = 1; // X Z Y order
			}
			else 
			{
				i1 = 0; 
				j1 = 0;
				k1 = 1;
				i2 = 1; 
				j2 = 0; 
				k2 = 1; // Z X Y order
			}
		}
		else 
		{ // x0<y0
			if (y0 < z0) 
			{
				i1 = 0; 
				j1 = 0; 
				k1 = 1; 
				i2 = 0; 
				j2 = 1; 
				k2 = 1; // Z Y X order
			}
			else if (x0 < z0) 
			{
				i1 = 0; 
				j1 = 1; 
				k1 = 0; 
				i2 = 0; 
				j2 = 1; 
				k2 = 1; // Y Z X order
			}
			else {
				i1 = 0; 
				j1 = 1; 
				k1 = 0; 
				i2 = 1; 
				j2 = 1; 
				k2 = 0; // Y X Z order
			}
		}

		// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
		// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
		// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
		// c = 1/6.
		float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
		float y1 = y0 - j1 + G3;
		float z1 = z0 - k1 + G3;
		float x2 = x0 - i2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
		float y2 = y0 - j2 + 2.0f * G3;
		float z2 = z0 - k2 + 2.0f * G3;
		float x3 = x0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
		float y3 = y0 - 1.0f + 3.0f * G3;
		float z3 = z0 - 1.0f + 3.0f * G3;

		auto hash = [](int32_t i) {
			return p[static_cast<uint8_t>(i)];
		};

		// Work out the hashed gradient indices of the four simplex corners
		int gi0 = hash(i + hash(j + hash(k)));
		int gi1 = hash(i + i1 + hash(j + j1 + hash(k + k1)));
		int gi2 = hash(i + i2 + hash(j + j2 + hash(k + k2)));
		int gi3 = hash(i + 1 + hash(j + 1 + hash(k + 1)));

		auto grad = [](int32_t hash, float x, float y, float z) {
			int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
			float u = h < 8 ? x : y; // gradient directions, and compute dot product.
			float v = h < 4 ? y : h == 12 || h == 14 ? x : z; // Fix repeats at h = 12 to 15
			return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
		};

		// Calculate the contribution from the four corners
		float t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
		if (t0 < 0) 
		{
			n0 = 0.0;
		}
		else 
		{
			t0 *= t0;
			n0 = t0 * t0 * grad(gi0, x0, y0, z0);
		}
		float t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
		if (t1 < 0) 
		{
			n1 = 0.0;
		}
		else 
		{
			t1 *= t1;
			n1 = t1 * t1 * grad(gi1, x1, y1, z1);
		}
		float t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
		if (t2 < 0) 
		{
			n2 = 0.0;
		}
		else 
		{
			t2 *= t2;
			n2 = t2 * t2 * grad(gi2, x2, y2, z2);
		}
		float t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
		if (t3 < 0)
		{
			n3 = 0.0;
		}
		else 
		{
			t3 *= t3;
			n3 = t3 * t3 * grad(gi3, x3, y3, z3);
		}
		// Add contributions from each corner to get the final noise value.
		// The result is scaled to stay just inside [-1,1]
		return 32.0f * (n0 + n1 + n2 + n3);
	}


	float octavePerlin(glm::vec3 pos, float persistence, int octaves)
	{
		float total = 0.0f;
		float frequency = 0.01f;
		float amplitude = 1.0f;
		float maxValue = 0.0f;

		for (uint32_t i = 0; i < octaves; i++)
		{
			total += perlin3D(pos.x * frequency, pos.y * frequency, pos.z * frequency) * amplitude;

			maxValue += amplitude;

			amplitude *= persistence;
			frequency *= 2;
		}

		return  total / maxValue;
	}

	float octaveSimplex(glm::vec3 pos, float persistence, int octaves)
	{
		float total = 0.0f;
		float frequency = 1.0f;
		float amplitude = 1.0f;
		float maxValue = 0.0f;

		for (uint32_t i = 0; i < octaves; i++)
		{
			total += simplex3D(pos.x * frequency, pos.y * frequency, pos.z * frequency) * amplitude;

			maxValue += amplitude;

			amplitude *= persistence;
			frequency *= 2;
		}

		return  total / maxValue;
	}
}

void GameModule::setType(Block& block)
{
}

BlockType getBlockType(Chunk& chunk, const glm::vec3& pos)
{
	float heightOffset = 80.0f;

	float densityMod = ((heightOffset - pos.y) / g_chunkSize.y) * 2;
	float density = Noise::octavePerlin(pos, 0.5f, 4);

	if (densityMod + density > 0.0f)
	//if (heightOffset > pos.y)
	{
		return BlockType::GRASS;
	}

	return BlockType::AIR;
}


Chunk GameModule::generateChunk(const glm::ivec3& pos)
{
	Chunk chunk;
	chunk.pos = pos;
	if (chunk.blocks.empty())
	{
		chunk.blocks.reserve(g_nBlocks);
	}

	for (int32_t y = 0; y < g_chunkSize.y; y++)
	{
		for (int32_t z = 0; z < g_chunkSize.z; z++)
		{
			for (int32_t x = 0; x < g_chunkSize.x; x++)
			{
				Block block;
				block.type = getBlockType(chunk, chunk.pos + glm::vec3(x, y, z));
				chunk.blocks.push_back(block);
			}
		}
	}

	return chunk;
}

int8_t getFaceId(BlockType type, Face::FaceType face)
{
	if (type == BlockType::GRASS)
	{
		switch (face)
		{
		case Face::FaceType::TOP:
			return 0;
		case Face::FaceType::BOTTOM:
			return 2;
		case Face::FaceType::RIGHT:
		case Face::FaceType::LEFT:
		case Face::FaceType::FRONT:
		case Face::FaceType::BACK:
			return 1;
		}
	}
	
	if (type == BlockType::DIRT)
	{
		return 2;
	}
}

void updateFace(Chunk& chunk, const glm::vec3 pos, BlockType type, Face::FaceType face)
{
	int8_t texID = getFaceId(type, face);

	for (uint32_t iVertex = 0; iVertex < g_vertexPerFace; iVertex++)
	{
		chunk.mesh.vertices.push_back({ 
				g_faces[face][iVertex].pos + pos,  
				g_faces[face][iVertex].coordInd,
				texID
			}
		);
	}
}

void GameModule::setBlockFace(Chunk& chunk, const glm::vec3& pos, BlockType type, Face::FaceType face)
{
	chunk.updated = false;
	updateFace(chunk, pos, type, face);
}

void GameModule::removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type)
{
	chunk.updated = false;
}

void GameModule::generateMesh(Chunk& chunk)
{

}

void GameModule::loadChunkMesh(Chunk& chunk)
{
	loadData(&chunk.mesh);
}

void GameModule::updateChunkNeighbourFace(Chunk& chunk1, Chunk& chunk2)
{
	auto& less = 
		chunk1.pos.x < chunk2.pos.x || chunk1.pos.z < chunk2.pos.z ? chunk1 : chunk2;
	auto& more = 
		chunk1.pos.x > chunk2.pos.x || chunk1.pos.z > chunk2.pos.z ? chunk1 : chunk2;
	if (less.pos.x < more.pos.x)
	{
		for (uint32_t y = 0; y < g_chunkSize.y; y++)
		{
			for (uint32_t z = 0; z < g_chunkSize.z; z++)
			{
				uint32_t iLess = 
					g_chunkSize.x * (y * g_chunkSize.z + z + 1) - 1;
				uint32_t iMore = 
					g_chunkSize.x * (y * g_chunkSize.z + z) + 0;

				if (less.blocks[iLess].type == BlockType::AIR &&
					more.blocks[iMore].type != BlockType::AIR)
				{
					setBlockFace(
						more,
						more.pos + glm::vec3(0, y, z),
						more.blocks[iMore].type,
						Face::FaceType::LEFT);
				}
				else if (less.blocks[iLess].type != BlockType::AIR &&
					more.blocks[iMore].type == BlockType::AIR)
				{
					setBlockFace(
						less, 
						less.pos + glm::vec3(g_chunkSize.x - 1, y, z),
						less.blocks[iLess].type,
						Face::FaceType::RIGHT);
				}
			}
		}
	}
	else if (less.pos.z < more.pos.z)
	{
		for (uint32_t y = 0; y < g_chunkSize.y; y++)
		{
			for (uint32_t x = 0; x < g_chunkSize.x; x++)
			{
				uint32_t iLess = 
					y * g_chunkSize.x * g_chunkSize.z + (g_chunkSize.z - 1) * g_chunkSize.x + x;
				uint32_t iMore = 
					y * g_chunkSize.x * g_chunkSize.z + (0) * g_chunkSize.x + x;

				if (less.blocks[iLess].type == BlockType::AIR &&
					more.blocks[iMore].type != BlockType::AIR)
				{
					setBlockFace(
						more, 
						more.pos + glm::vec3(x, y, 0), 
						more.blocks[iMore].type,
						Face::FaceType::BACK);
				}
				else if (less.blocks[iLess].type != BlockType::AIR &&
					more.blocks[iMore].type == BlockType::AIR)
				{
					setBlockFace(
						less,
						less.pos + glm::vec3(x, y, g_chunkSize.z - 1),
						less.blocks[iLess].type,
						Face::FaceType::FRONT);
				}
			}
		}
	}
}

void GameModule::drawChunk(const Chunk& chunk)
{
	if (chunk.mesh.vertices.size() == 0)
	{
		return;
	}

	renderMesh(&chunk.mesh);
}

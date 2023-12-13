#pragma once

namespace Engine
{
	namespace Noise
	{
		float perlin2D(float x, float y);
		float perlin3D(float x, float y, float z);
		
		float octavePerlin3D(const glm::vec3& pos, float persistence, int octaves);
		float octavePerlin2D(const glm::vec3& pos, float persistence, int octaves);
		float octaveRigid2D(const glm::vec3& pos, float persistence, int octaves);
		float octaveTerraces(const glm::vec3& pos, float persistence, int octaves);
	}
}
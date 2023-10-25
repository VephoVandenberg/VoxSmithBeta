#pragma once

#include "../engine/shader/shader.h"
#include "../engine/texture/texture.h"
#include "../engine/camera/camera.h"

#ifdef ECS
#include "../modules/chunk/block.h"

#include <queue>
#else
#include "../modules/chunk/chunk.h"
#endif


#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>
#include <thread>

struct GLFWwindow;

namespace App
{
	class Application
	{
	public:
		Application();
		~Application();

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;

		void run();

	private:
		void init();
		void initShaders();
		void initTextures();
		void onRender();
		void onUpdate();
		void handleInput();
		void updateTerrain();
		void processRay(Engine::Ray ray);

		bool									m_isRunning = false;
		bool									m_keyboard[1024];
		bool									m_keyboardPressed[1024];
		GLFWwindow*								m_window = nullptr;

		glm::ivec3								m_minBorder;
		glm::ivec3								m_maxBorder;

		std::map<const char*, Engine::Shader>	m_shaders;
		std::map<const char*, Engine::Texture>	m_textures;

#ifdef ECS
		std::queue<int32_t> m_availableEntities;

		std::unordered_map<int32_t, int32_t> m_entityToIndex;
		std::unordered_map<int32_t, int32_t> m_indexToEntity;

		std::vector<GameModule::Block> m_components;
		uint32_t m_livingEntity = 0;
#else
		struct KeyFuncs
		{
			size_t operator()(const glm::ivec3& v)const
			{
				return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
			}

			bool operator()(const glm::ivec3& a, const glm::vec3& b)const
			{
				return a.x == b.x && a.z == b.z;
			}
		};

		std::unordered_map<glm::ivec3, GameModule::Chunk, KeyFuncs> m_chunks;
		std::vector<std::thread> m_threads;
#endif
	};
}

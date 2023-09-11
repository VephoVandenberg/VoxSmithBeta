#pragma once

#include "../engine/shader/shader.h"
#include "../engine/texture/texture.h"
#include "../engine/camera/camera.h"

#include "../modules/chunk/chunk.h"

#include <map>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

namespace App
{
	class Application
	{
	public:
		Application();
		~Application();

		void run();
		
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;

	private:
		void init();
		void initShaders();
		void initTextures();
		void onRender();
		void onUpdate();
		void handleInput();

		bool m_isRunning = false;
		bool m_keyboard[1024];
		bool m_keyboardPressed[1024];
		GLFWwindow* m_window = nullptr;

		std::map<const char*, Engine::Shader> m_shaders;
		std::map<const char*, Engine::Texture> m_textures;

#ifdef GLOBAL_BLOCKS
		std::vector<GameModule::Block> m_blocks;
#else
		struct KeyFuncs
		{
			size_t operator()(const glm::vec3& v)const
			{
				return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
			}

			bool operator()(const glm::vec3& a, const glm::vec3& b)const
			{
				return a.x == b.x && a.z == b.z;
			}
		};

		std::unordered_map<glm::vec3, GameModule::Chunk, KeyFuncs> m_chunks;
#endif
	};
}

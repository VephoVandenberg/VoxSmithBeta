#pragma once

#include "../engine/shader/shader.h"
#include "../engine/texture/texture.h"
#include "../engine/camera/camera.h"

#ifdef ECS
#include "../modules/chunk/block.h"

#include <queue>
#else
#include "../modules/world/world.h"
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
		//void updateTerrain();
		//void processRay(Engine::Ray ray);
		//void traceRay(glm::vec3 rayPosFrac, GameModule::RayType type);
		
		bool									m_isRunning = false;
		bool									m_keyboard[1024];
		bool									m_keyboardPressed[1024];
		GLFWwindow*								m_window = nullptr;

		// Move to world

		std::map<const char*, Engine::Shader>	m_shaders;
		std::map<const char*, Engine::Texture>	m_textures;

#ifdef ECS
#else
		GameModule::World m_world;
		
		std::vector<std::thread> m_threads;
#endif
	};
}

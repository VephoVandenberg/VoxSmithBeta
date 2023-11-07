#pragma once

#include "../engine/shader/shader.h"
#include "../engine/texture/texture.h"
#include "../engine/camera/camera.h"
#include "../engine/ray/ray.h"
#include "../engine/renderer/block_renderer.h"

#ifdef ECS
#endif

#include "../modules/player/player.h"
#include "../modules/world/world.h"

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
		void initPlayer();
		void onRender();
		void onUpdate(float dt);
		void handleCamera(const double xPos, const double yPos);
		void handleInput();
		
		bool									m_isRunning = false;
		bool									m_keyboard[1024];
		bool									m_keyboardPressed[1024];
		GLFWwindow*								m_window = nullptr;

		std::map<const char*, Engine::Shader>	m_shaders;
		std::map<const char*, Engine::Texture>	m_textures;
		Engine::TextureArray					m_tArray;

		GameModule::World						m_world;
		GameModule::Player						m_player;

		Engine::Ray								m_ray;
		Engine::Renderer::Buffer				m_rayBuffer;
		
		std::vector<std::thread>				m_threads;

		friend void cursorCallbackWrapper(GLFWwindow* window, const double x, const double y);
	};
}

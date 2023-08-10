#pragma once

#include "../engine/shader/shader.h"
#include "../engine/texture/texture.h"

#include <map>

struct GLFWwindow;

namespace App
{
	class Application
	{
	public:
		Application();

		void run();

		~Application() = default;
		
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;

	private:
		void init();
		void handleInput();
		void onRender();
		void onUpdate();

		bool m_isRunning = false;
		GLFWwindow* m_window = nullptr;

		std::map<const char*, Engine::Shader> m_shaders;
		std::map<const char*, Engine::Texture> m_textures;
	};
}

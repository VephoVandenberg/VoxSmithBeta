#pragma once

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

		bool m_isRunning = false;
		GLFWwindow* m_window = nullptr;
	};
}
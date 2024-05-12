#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>
#include <mutex>
#include <thread>
#include <algorithm>

#include "../engine/window/window.h"
#include "../engine/shader/shader_list.h"
#include "../engine/texture/texture_list.h"
#include "../engine/texture/framebuffer.h"

#include "../engine/ray/ray.h"
#include "../engine/renderer/block_renderer.h"

#include "../modules/chunk/block.h"

#include "../modules/chunk/chunk.h"

#include "app.h"

using namespace Engine;
using namespace GameModule;
using namespace App;


const char* g_title = "VoxSmithBeta";
constexpr size_t g_width = 1600;
constexpr size_t g_height = 900;

constexpr size_t g_numberOfChunksX = 24;
constexpr size_t g_numberOfChunksZ = 24;

constexpr size_t g_jumpHeight = 3;

constexpr float g_playerSpeed = 10.0f;
constexpr float g_playerAcceleration = 30.0f;

constexpr float g_gravity = 0.9f;
constexpr float g_jumpForce = 12.0f;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

#ifdef _DEBUG
bool g_showCascades = false;
int32_t g_layer = 0;
bool g_debugShadow = false;
#endif

void App::cursorCallbackWrapper(GLFWwindow* window, const double x, const double y)
{
	static_cast<Application*>(glfwGetWindowUserPointer(window))->handleCamera(x, y);
}

Application::Application()
	: m_isRunning(true)
{
	init();
}

Application::~Application()
{
}

void Application::init()
{
	m_window = getWindow(g_title, g_width, g_height);

	initPlayer();

	initShaders();
	initTextures();

	initWorld(m_world, m_player);
}

void Application::initPlayer()
{
	m_player.height = 2.0f;
	m_player.pos = {
		g_chunkSize.x * g_chunksX / 2,
		g_chunkSize.y / 2.0f + 30.0f,
		g_chunkSize.z * g_chunksZ / 2,
	};
	m_player.velocity = { 0.0f, 0.0f, 0.0f };
	m_player.size = { 0.6f, 2.0f, 0.6f };

	m_player.speed = 400;
	m_player.isJumping = false;
	m_player.camera.lastX = 620;
	m_player.camera.lastY = 360;
	m_player.camera.yaw = -90.0f;
	m_player.camera.pitch = 0.0f;
	m_player.camera.pos = {
		m_player.pos.x + m_player.size.x / 2.0f,
		m_player.pos.y + m_player.height - 0.2f,
		m_player.pos.z + m_player.size.z / 2.0f };
	m_player.camera.speed = 1.0f;
	m_player.camera.front = { 0.0f, 0.0f, 1.0f };
	m_player.camera.up = { 0.0f, 1.0f, 0.0f };
	m_player.camera.view =
		glm::lookAt(
			m_player.camera.pos,
			m_player.camera.front,
			m_player.camera.up);
	m_player.camera.nearPlane = 0.1f;
	m_player.camera.farPlane = 200.0f;

	glfwSetWindowUserPointer(m_window, this);

	glfwSetCursorPosCallback(m_window, cursorCallbackWrapper);
}

void Application::initShaders()
{
	Engine::Renderer::loadCubeData();
	Engine::Renderer::loadPlayerOutlineData();
	Engine::Renderer::loadQuadData();

	Engine::loadShaders(m_shaders);

	glm::mat4 projection =
		glm::perspective(
			glm::radians(45.0f),
			static_cast<float>(g_width) / static_cast<float>(g_height),
			m_player.camera.nearPlane, m_player.camera.farPlane);

	m_player.camera.projection = projection;

	useShader(m_shaders[ShadersAvailable::s_cubeShader]);
	setUniform4m(m_shaders[ShadersAvailable::s_cubeShader], "u_projection", projection);
	setUniform4m(m_shaders[ShadersAvailable::s_cubeShader], "u_view", m_player.camera.view);

	useShader(m_shaders[ShadersAvailable::s_meshAndShadow]);
	setUniform4m(m_shaders[ShadersAvailable::s_meshAndShadow], "u_projection", projection);
	setUniform4m(m_shaders[ShadersAvailable::s_meshAndShadow], "u_view", m_player.camera.view);

	useShader(m_shaders[ShadersAvailable::s_lightObj]);
	setUniform4m(m_shaders[ShadersAvailable::s_lightObj], "u_projection", projection);
	setUniform3f(m_shaders[ShadersAvailable::s_lightObj], "u_position", m_world.lightDir);
	setUniform4m(m_shaders[ShadersAvailable::s_lightObj], "u_view", m_player.camera.view);

	useShader(m_shaders[ShadersAvailable::s_rayShader]);
	setUniform4m(m_shaders[ShadersAvailable::s_rayShader], "u_projection", projection);
	setUniform4m(m_shaders[ShadersAvailable::s_rayShader], "u_view", m_player.camera.view);

	useShader(m_shaders[ShadersAvailable::s_outlineShader]);
	setUniform4m(m_shaders[ShadersAvailable::s_outlineShader], "u_projection", projection);
	setUniform4m(m_shaders[ShadersAvailable::s_outlineShader], "u_view", m_player.camera.view);
}

void Application::initTextures()
{
	initTextureArray(m_tArray, s_tPaths);
}

void Application::run()
{
	static float limits = 1.0f / 60.0f;
	float previousFrame = (float)glfwGetTime();
	while (m_isRunning)
	{
		float currentFrame = (float)glfwGetTime();
		float dt = (currentFrame - previousFrame);
		previousFrame = currentFrame;

		onUpdate(dt);
		clearScreen();

		onRender();

		handleInput();
		updateScreen(m_window);

	}
}

void Application::handleInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_W] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_W] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_A] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_A] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_D] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_D] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_S] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_S] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_SPACE] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_SPACE] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_F) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_KEY_F])
	{
		m_keyboard[GLFW_KEY_F] = true;
	}
	else if (glfwGetKey(m_window, GLFW_KEY_F) == GLFW_RELEASE)
	{
		m_keyboard[GLFW_KEY_F] = false;
		m_keyboardPressed[GLFW_KEY_F] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_N) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_KEY_N])
	{
		m_keyboard[GLFW_KEY_N] = true;
	}
	else if (glfwGetKey(m_window, GLFW_KEY_N) == GLFW_RELEASE)
	{	
		m_keyboard[GLFW_KEY_N] = false;
		m_keyboardPressed[GLFW_KEY_N] = false;
	}

#ifdef _DEBUG
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_KEY_R])
	{
		g_showCascades = true;
	}
	else if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_RELEASE)
	{
		g_showCascades = false;
	}
#endif


	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT])
	{
		m_keyboard[GLFW_MOUSE_BUTTON_LEFT] = true;
	}
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		m_keyboard[GLFW_MOUSE_BUTTON_LEFT] = false;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = false;
	}


	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT])
	{
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] = true;
	}
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] = false;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = false;
	}
}

void Application::onRender()
{
	setUniform4m(m_shaders[ShadersAvailable::s_lightObj], "u_view", m_player.camera.view);
	//Renderer::render(Renderer::Type::CUBE);

	setUniform4m(m_shaders[ShadersAvailable::s_outlineShader], "u_view", m_player.camera.view);
	//Renderer::render(Renderer::Type::CUBE_LINES);

	setUniform4m(m_shaders[ShadersAvailable::s_outlineShader], "u_view", m_player.camera.view);
	setUniform3f(m_shaders[ShadersAvailable::s_outlineShader], "u_position", m_player.pos);
	//Renderer::render(Renderer::Type::PLAYER);

	drawWorlToSM(m_world, m_player, m_shaders[ShadersAvailable::s_cascadedDepth]);
	Engine::setViewport(g_width, g_height);
	Engine::clearBuffers();

#ifdef _DEBUG
	if (g_debugShadow)
	{
		setUniformi(m_shaders[ShadersAvailable::s_debugQuad], "u_depthMap", 1);
		setUniformi(m_shaders[ShadersAvailable::s_debugQuad], "u_layer", g_layer);
		drawDebugQuad(m_world, m_shaders[ShadersAvailable::s_debugQuad]);
	}
	else
	{
		setUniform4m(m_shaders[ShadersAvailable::s_meshAndShadow], "u_view", m_player.camera.view);
		setUniformi(m_shaders[ShadersAvailable::s_meshAndShadow], "u_textureArray", 0);
		useTextureArray(m_tArray);
		setUniformi(m_shaders[ShadersAvailable::s_meshAndShadow], "u_shadowMap", 1);
		setUniformBool(m_shaders[ShadersAvailable::s_meshAndShadow], "u_showCascades", g_showCascades);
		drawWorld(m_world, m_player, m_shaders[ShadersAvailable::s_meshAndShadow]);
	}
	setUniform4m(m_shaders[ShadersAvailable::s_rayShader], "u_view", m_player.camera.view);
	//Renderer::render(Renderer::Type::RAY);
#else
	setUniform4m(m_shaders[ShadersAvailable::s_meshAndShadow], "u_view", m_player.camera.view);
	setUniformi(m_shaders[ShadersAvailable::s_meshAndShadow], "u_textureArray", 0);
	setUniformi(m_shaders[ShadersAvailable::s_meshAndShadow], "u_shadowMap", 1);
	useTextureArray(m_tArray);
	drawWorld(m_world, m_player, m_shaders[ShadersAvailable::s_meshAndShadow]);
#endif
}

void Application::onUpdate(float dt)
{
	Ray ray = castRay(m_player.camera);
	RayType type = RayType::IDLE;

	// Ray handling
	if (m_keyboard[GLFW_MOUSE_BUTTON_LEFT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] ||
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT])
	{
		Renderer::loadRayData(ray);

		type = m_keyboard[GLFW_MOUSE_BUTTON_LEFT] ? RayType::REMOVE : RayType::PLACE;

		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = true;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = true;
	}

	static bool move = false;
	// Player handling
	glm::vec3 v = glm::normalize(
		glm::vec3(m_player.camera.front.x, m_player.camera.front.y, m_player.camera.front.z));
	if (m_keyboard[GLFW_KEY_W])
	{
		m_player.velocity += v * m_player.speed * dt;
	}

	if (m_keyboard[GLFW_KEY_S])
	{
		m_player.velocity += -v * m_player.speed * dt;
	}

	if (m_keyboard[GLFW_KEY_A])
	{
		auto left = glm::normalize(glm::cross(v, glm::vec3(0.0f, 1.0f, 0.0f)));
		m_player.velocity += -left * m_player.speed * dt;
	}

	if (m_keyboard[GLFW_KEY_D])
	{
		auto right = glm::normalize(glm::cross(v, glm::vec3(0.0f, 1.0f, 0.0f)));
		m_player.velocity += right * m_player.speed * dt;
	}

	// My shitty jumping
	if (m_keyboard[GLFW_KEY_SPACE] && !m_player.isJumping)
	{
		m_player.isJumping = true;
		m_player.jumpAcceleration = 1.5f;
		m_player.heightJumped = 0.0f;
	}

#ifdef _DEBUG
	if (m_keyboard[GLFW_KEY_F] && !m_keyboardPressed[GLFW_KEY_F])
	{
		g_debugShadow = !g_debugShadow;
		g_layer = 0;
		m_keyboardPressed[GLFW_KEY_F] = true;
	}

	if (m_keyboard[GLFW_KEY_N] && !m_keyboardPressed[GLFW_KEY_N])
	{
		g_layer++;
		m_keyboardPressed[GLFW_KEY_N] = true;
		if (g_layer > 5)
		{
			g_layer = 0;
		}

	}
#endif

	glm::vec3 up = glm::vec3(0.0f, 0.1f, 0.0f);
	glm::vec3 down = glm::vec3(0.0f, -0.1f, 0.0f);
	if (m_player.heightJumped < g_jumpHeight)
	{
		m_player.heightJumped += up.y * m_player.jumpAcceleration * dt;
		//m_player.velocity += up * m_player.jumpAcceleration * dt;
		m_player.jumpAcceleration -= g_gravity * down.y * dt;
	}

	//if (isPlayerFalling(m_world, m_player, dt))
	{
		//	m_player.velocity += down * g_gravity * dt;
	}
	//else
	{
		if (!m_keyboard[GLFW_KEY_SPACE] && m_player.isJumping)
		{
			m_player.isJumping = false;
		}
	}

	//checkPlayerCollision(m_world, m_player, dt);

	m_player.camera.pos += m_player.velocity * dt;
	m_player.pos += m_player.velocity * dt;

	//m_world.fractionPos += m_player.velocity * dt;
	//m_world.pos = static_cast<glm::ivec3>(m_world.fractionPos) / 16 * 16;
	m_player.velocity *= 0.95f;

	updateWorld(m_world, m_player, dt);
	updateCameraView(m_player.camera);
	//processRay(m_world, m_player, ray, m_shaders[s_outlineShader], type);
}

void Application::handleCamera(const double xPos, const double yPos)
{
	static bool firstMove = true;
	if (firstMove)
	{
		m_player.camera.lastX = xPos;
		m_player.camera.lastY = yPos;
		firstMove = false;
	}

	float xoffset = xPos - m_player.camera.lastX;
	float yoffset = m_player.camera.lastY - yPos;
	m_player.camera.lastX = xPos;
	m_player.camera.lastY = yPos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	m_player.camera.yaw += xoffset;
	m_player.camera.pitch += yoffset;

	if (m_player.camera.pitch > 89.0f)
	{
		m_player.camera.pitch = 89.0f;
	}
	if (m_player.camera.pitch < -89.0f)
	{
		m_player.camera.pitch = -89.0f;
	}

	glm::vec3 direction;
	direction.x = cos(glm::radians(m_player.camera.yaw)) * cos(glm::radians(m_player.camera.pitch));
	direction.y = sin(glm::radians(m_player.camera.pitch));
	direction.z = sin(glm::radians(m_player.camera.yaw)) * cos(glm::radians(m_player.camera.pitch));
	m_player.camera.front = glm::normalize(direction);
}

#pragma once

#include <glad/glad.h>
#include <iostream>

#include "framebuffer.h"

using namespace Engine;

constexpr size_t g_widthResolution = 4096;
constexpr size_t g_heightResolution = 4096;

void Engine::initFBuffer(FBuffer& fBuffer) 
{
	glGenFramebuffers(1, &fBuffer.ID);
	
	glGenTextures(1, &fBuffer.map);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fBuffer.map);
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_DEPTH_COMPONENT32, 
		g_widthResolution, 
		g_heightResolution, 
		0, 
		GL_DEPTH_COMPONENT, 
		GL_FLOAT, 
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.ID);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, 
		GL_DEPTH_ATTACHMENT, 
		GL_TEXTURE_2D,
		fBuffer.map,
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		exit(EXIT_FAILURE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::initFArrayBuffer(FBuffer& fBuffer, uint32_t n, const std::vector<glm::vec3>& cascades)
{
	glGenFramebuffers(1, &fBuffer.ID);

	glGenTextures(1, &fBuffer.map);
	glBindTexture(GL_TEXTURE_2D_ARRAY, fBuffer.map);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		GL_DEPTH_COMPONENT32,
		g_widthResolution,
		g_heightResolution,
		cascades.size()  + 1,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.ID);

	constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.ID);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fBuffer.map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		exit(EXIT_FAILURE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::bindFBuffer(FBuffer& fBuffer) 
{
	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.ID);
}

void Engine::unbindFBuffer() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::setFramebufferViewport()
{
	Engine::setViewport(g_widthResolution, g_heightResolution);
}

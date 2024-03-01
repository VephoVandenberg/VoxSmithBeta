#pragma once

#include <glad/glad.h>
#include <iostream>

#include "framebuffer.h"

using namespace Engine;

constexpr uint32_t g_shadowResolution = 3072;

void Engine::initFBuffer(FBuffer& fBuffer) 
{
	glGenFramebuffers(1, &fBuffer.id);
	
	glGenTextures(1, &fBuffer.map);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fBuffer.map);
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_DEPTH_COMPONENT32, 
		g_shadowResolution,
		g_shadowResolution,
		0, 
		GL_DEPTH_COMPONENT, 
		GL_FLOAT, 
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.id);
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
		std::cout << "ERROR::FRAMEBUFFER::FRAMEBUFFER_NOT_COMPLETE" << std::endl;
		exit(EXIT_FAILURE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::initFArrayBuffer(FBuffer& fBuffer, const std::vector<float>& cascades)
{
	glGenFramebuffers(1, &fBuffer.id);

	glGenTextures(1, &fBuffer.map);
	glBindTexture(GL_TEXTURE_2D_ARRAY, fBuffer.map);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		GL_DEPTH_COMPONENT32F,
		g_shadowResolution,
		g_shadowResolution,
		cascades.size() + 1,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.id);
	glFramebufferTexture(
		GL_FRAMEBUFFER, 
		GL_DEPTH_ATTACHMENT,
		fBuffer.map, 
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER::FRAMEBUFFER_NOT_COMPLETE" << std::endl;
		exit(EXIT_FAILURE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::useFArray(FBuffer& fBuffer)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, fBuffer.map);
}

void Engine::bindFBuffer(FBuffer& fBuffer) 
{
	glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.id);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, fBuffer.map, 0);
}

void Engine::unbindFBuffer() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::setFramebufferViewport()
{
	Engine::setViewport(g_shadowResolution, g_shadowResolution);
}

void Engine::cullFront()
{
	glCullFace(GL_FRONT);
}

void Engine::cullBack()
{
	glCullFace(GL_BACK);
}

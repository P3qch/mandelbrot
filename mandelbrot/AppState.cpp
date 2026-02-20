#include <cmath>
#include <algorithm>
#include "AppState.h"
#include "Utils.h"
#include "Mandelbrot.h"
#include "Exceptions.h"

AppState::AppState(const int width, const int height)
	: m_context(SDL_INIT_VIDEO | SDL_INIT_EVENTS), 
	m_gpuDevice(SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr)),
	m_window(SDL_CreateWindow("Mandelbrot Explorer", width, height, 0))
{
	if (NULL == m_gpuDevice)
	{
		throw SdlException("Coulndt get gpu device");
	}
	if (NULL == m_window.get())
	{
		throw SdlException("Failed to create window");
	}

	if (!SDL_ClaimWindowForGPUDevice(m_gpuDevice.get(), m_window.get()))
	{
		SDL_Log("uhhh %s\n", SDL_GetError());
		throw SdlException("GPUClaimWindow failed");
	}

	prepareGraphicsPipeline();
	uploadVertices();
}

void AppState::update()
{
	if (!m_updateNeeded)
	{
		return;
	}
	m_updateNeeded = false;

	SDL_Log("Start\n");

	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(m_gpuDevice.get());
	SDL_GPUTexture* swapchainTexture = nullptr;
	Uint32 swapchainWidth = 0;
	Uint32 swapchainHeight = 0;



	SDL_WaitAndAcquireGPUSwapchainTexture(
		cmdBuffer,
		m_window.get(),
		&swapchainTexture,
		&swapchainWidth,
		&swapchainHeight
	);

	SDL_GPUColorTargetInfo colorTargetInfo = { };
	colorTargetInfo.texture = swapchainTexture;
	colorTargetInfo.clear_color = SDL_FColor { 0.0f, 0.0f, 0.0f, 1.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTargetInfo, 1, NULL);
	SDL_BindGPUGraphicsPipeline(renderPass, m_graphicsPipeline.get());
	SDL_GPUBufferBinding vertexBinding = { m_verticesBuffer, 0 };
	SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
	SDL_GPUBufferBinding indicesBinding = { m_indicesBuffer, 0 };
	SDL_BindGPUIndexBuffer(renderPass, &indicesBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

	double res[] = { m_minX, m_maxX, m_minY, m_maxY };
	SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &res, sizeof(res));

	SDL_DrawGPUIndexedPrimitives(renderPass, 6, 2, 0, 0, 0);

	SDL_EndGPURenderPass(renderPass);
	SDL_SubmitGPUCommandBuffer(cmdBuffer);


	SDL_Log("Done\n");
}

void AppState::handleEvent(SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_EVENT_QUIT:
		throw QuitException();

	case SDL_EVENT_KEY_DOWN:
		long double xWidth = m_maxX - m_minX;
		long double yWidth = m_maxY - m_minY;
		m_updateNeeded = true;
		switch (event->key.key)
		{
		case SDLK_LEFT:
			m_minX -= xWidth * long double(0.05);
			m_maxX -= xWidth * long double(0.05);
			break;
		case SDLK_RIGHT:
			m_minX += xWidth * long double(0.05);
			m_maxX += xWidth * long double(0.05);
			break;
		case SDLK_UP:
			m_minY -= yWidth * long double(0.05);
			m_maxY -= yWidth * long double(0.05);
			break;
		case SDLK_DOWN:
			m_minY += yWidth * long double(0.05);
			m_maxY += yWidth * long double(0.05);
			break;
		case SDLK_EQUALS:
		case SDLK_PLUS:
			m_minX += xWidth * long double(0.1);
			m_maxX -= xWidth * long double(0.1);
			m_minY += yWidth * long double(0.1);
			m_maxY -= yWidth * long double(0.1);
			break;
		case SDLK_MINUS:
			m_minX -= xWidth * long double(0.5);
			m_maxX += xWidth * long double(0.5);
			m_minY -= yWidth * long double(0.5);
			m_maxY += yWidth * long double(0.5);
			break;
		default:
			m_updateNeeded = true;
			break;
		}
		break;
	}
}

SdlShaderPtr AppState::loadShader(SDL_GPUDevice* device, const char* path, SDL_GPUShaderStage stage, const char* entryPoint)
{
	size_t codeSize = 0;
	void* shaderCode = SDL_LoadFile(path, &codeSize);
	if (!shaderCode)
		throw std::runtime_error("Failed to load shader file " );

	SDL_GPUShaderCreateInfo createInfo = {};
	createInfo.code_size = codeSize;
	createInfo.code = reinterpret_cast<const Uint8*>(shaderCode);
	createInfo.entrypoint = entryPoint;
	createInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	createInfo.stage = stage;
	createInfo.num_uniform_buffers = 1;

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &createInfo);

	SDL_free(shaderCode); // cleanup the loaded file buffer

	if (!shader)
		throw std::runtime_error("Failed to create GPU shader");

	return SdlShaderPtr(
		shader, 
		[dev = device](SDL_GPUShader* s) 
		{
			if (s) SDL_ReleaseGPUShader(dev, s); 
		});
}

void AppState::prepareGraphicsPipeline()
{
	SdlShaderPtr vertShader = loadShader(
		m_gpuDevice.get(),
		"passthrough.vert.spv",
		SDL_GPU_SHADERSTAGE_VERTEX);

	SdlShaderPtr fragShader = loadShader(
		m_gpuDevice.get(),
		"mandelbrot.frag.spv",
		SDL_GPU_SHADERSTAGE_FRAGMENT);

	SDL_GPUColorTargetDescription colorDesc = {};
	colorDesc.format = SDL_GetGPUSwapchainTextureFormat(m_gpuDevice.get(), m_window.get());

	SDL_GPUVertexAttribute attr = { 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0 };
	SDL_GPUVertexBufferDescription bufferDesc = { 0, sizeof(float) * 2, SDL_GPU_VERTEXINPUTRATE_VERTEX, 0};

	SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = { 0 };
	pipelineInfo.vertex_shader = vertShader.get();
	pipelineInfo.fragment_shader = fragShader.get();
	pipelineInfo.target_info.color_target_descriptions = &colorDesc;
	pipelineInfo.target_info.num_color_targets = 1;
	pipelineInfo.vertex_input_state.num_vertex_attributes = 1;
	pipelineInfo.vertex_input_state.vertex_attributes = &attr;
	pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
	pipelineInfo.vertex_input_state.vertex_buffer_descriptions = &bufferDesc;

	m_graphicsPipeline = SdlGraphicsPipelinePtr(
		SDL_CreateGPUGraphicsPipeline(m_gpuDevice.get(), &pipelineInfo),
		[dev = m_gpuDevice.get()](SDL_GPUGraphicsPipeline* p)
		{
			if (p) SDL_ReleaseGPUGraphicsPipeline(dev, p);
		});

	if (m_graphicsPipeline.get() == nullptr)
	{
		throw SdlException("Something fucked up when creating pipeline");
	}
}

void AppState::uploadVertices()
{
	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(m_gpuDevice.get());
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	float vertices[] = {
		-1, -1,
		1, -1,
		1,  1,
		-1,  1
	};
	Uint16 indices[] = { 0, 1, 2, 2, 3, 0 };


	SDL_GPUBufferCreateInfo verticesInfo = { SDL_GPU_BUFFERUSAGE_VERTEX, sizeof(vertices) };
	m_verticesBuffer = SDL_CreateGPUBuffer(m_gpuDevice.get(), &verticesInfo);

	SDL_GPUBufferCreateInfo indicesInfo = { SDL_GPU_BUFFERUSAGE_INDEX, sizeof(indices) };
	m_indicesBuffer = SDL_CreateGPUBuffer(m_gpuDevice.get(), &indicesInfo);


	SDL_GPUTransferBufferCreateInfo transferInfo = { 
		SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, 
		sizeof(indices) + sizeof(vertices) 
	};
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(m_gpuDevice.get(), &transferInfo);

	void* mappedTransferBuffer = SDL_MapGPUTransferBuffer(m_gpuDevice.get(), transferBuffer, false);

	SDL_memcpy(mappedTransferBuffer, vertices, sizeof(vertices));
	SDL_memcpy(reinterpret_cast<char*>(mappedTransferBuffer) + sizeof(vertices), indices, sizeof(indices));

	SDL_UnmapGPUTransferBuffer(m_gpuDevice.get(), transferBuffer);

	SDL_GPUTransferBufferLocation transferBufferLocation = { transferBuffer, 0 };
	SDL_GPUBufferRegion bufferRegion = { m_verticesBuffer, 0, sizeof(vertices) };
	SDL_UploadToGPUBuffer(copyPass, &transferBufferLocation, &bufferRegion, false);

	transferBufferLocation.offset += sizeof(vertices);
	bufferRegion.buffer = m_indicesBuffer;
	bufferRegion.size = sizeof(indices);
	SDL_UploadToGPUBuffer(copyPass, &transferBufferLocation, &bufferRegion, false);


	SDL_ReleaseGPUTransferBuffer(m_gpuDevice.get(), transferBuffer);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(cmdBuffer);
}

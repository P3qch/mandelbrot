#pragma once
#include <SDL3/SDL.h>
#include <functional>
#include "Utils.h"


#define _SILENCE_NONFLOATING_COMPLEX_DEPRECATION_WARNING 1

using SdlGpuDevice = custom_unique_ptr<SDL_GPUDevice, &SDL_DestroyGPUDevice>;
using SdlWindow = custom_unique_ptr<SDL_Window, &SDL_DestroyWindow>;
using SdlShaderPtr = std::unique_ptr<SDL_GPUShader, std::function<void(SDL_GPUShader*)>>;
using SdlGraphicsPipelinePtr = std::unique_ptr<SDL_GPUGraphicsPipeline, std::function<void(SDL_GPUGraphicsPipeline*)>>;

using SdlGpuBuffer = std::unique_ptr<SDL_GPUBuffer, std::function<void(SDL_GPUBuffer*)>>;

class AppState
{
public:
	AppState(const int width = 1000, const int height = 1000);

	void update();
	void handleEvent(SDL_Event* event);

private:
	static SdlShaderPtr loadShader(SDL_GPUDevice* device,
		const char* path,
		SDL_GPUShaderStage stage,
		const char* entryPoint = "main");

	void prepareGraphicsPipeline();
	void uploadVertices();

	SdlContext m_context;
	SdlGpuDevice m_gpuDevice;
	SdlWindow m_window;
	SdlGraphicsPipelinePtr m_graphicsPipeline;


	SDL_GPUBuffer* m_verticesBuffer = nullptr;
	SDL_GPUBuffer* m_indicesBuffer = nullptr;

	double m_minX = -2.0;
	double m_maxX = 2.0;

	double m_minY = -2.0;
	double m_maxY = 2.0;

	bool m_updateNeeded = true;
};
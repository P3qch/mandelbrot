#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "AppState.h"
#include "Exceptions.h"

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
	try
	{
		*appstate = new AppState();
	}
	catch (const SdlException& e)
	{
		SDL_Log(e.what());
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}	

SDL_AppResult SDL_AppIterate(void* appstate)
{
	auto state = reinterpret_cast<AppState*>(appstate);

	try
	{
		state->update();
	}
	catch (const SdlException&)
	{

	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	auto state = reinterpret_cast<AppState*>(appstate);

	try
	{
		state->handleEvent(event);
	}
	catch (const QuitException&)
	{
		return SDL_APP_SUCCESS;
	}
	catch (const SdlException& e)
	{

	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	delete (AppState*)appstate;
}

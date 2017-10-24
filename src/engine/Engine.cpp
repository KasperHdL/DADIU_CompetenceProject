#include <engine/Engine.hpp>

#include <game/Game.hpp>
#include <debug/DebugInterface.hpp>
#include <engine/renderer/VRRenderer.hpp>


Engine::Engine(int screen_width, int screen_height){
    this->screen_width = screen_width;
    this->screen_height = screen_height;
}

Engine::~Engine(){
    
}

int Engine::initialize(Game* game){
    this->game = game;

    SDL_Init(SDL_INIT_VIDEO); // Initialize SDL2

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    // Create an application window with the following settings:
    window = SDL_CreateWindow(
            "DADIU Competence Project",                     // window title
            SDL_WINDOWPOS_UNDEFINED,              // initial x position
            SDL_WINDOWPOS_UNDEFINED,              // initial y position
            screen_width,                                  // width, in pixels
            screen_height,                                  // height, in pixels
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE// flags
    );

    // Check that the window was successfully made
    if (window == NULL) {
        // In the event that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

	VRRenderer *vr_renderer = new VRRenderer();
	vr_renderer->initialize(window);

	input.set_chaperone(vr::VRChaperone());

	input.vr_hmd = vr_renderer->m_pHMD;

    game->initialize(this);


	SDL_ShowCursor(SDL_DISABLE);

    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;

    while (!input.quit){

        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();

        delta_time = clamp(((NOW - LAST) / (float)SDL_GetPerformanceFrequency() ),0.0f,1.0f);
        time += delta_time;

		vr_renderer->HandleInput();
        update(delta_time);
		
		vr_renderer->RenderFrame();

		input.update_vr_pose();
    }

	SDL_StopTextInput();

    AssetManager::cleanup();

	vr_renderer->Shutdown();

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}


void Engine::update(float delta_time){
    input.update();
    
    game->update(delta_time);

    AssetManager::update();

}


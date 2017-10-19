#pragma once

//stl
#include <iostream>


//sdl
#include <SDL.h>
#undef main

//glm
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

//render
#include <engine/renderer/Renderer.hpp>

#include <openvr.h>

//engine
#include <engine/Input.hpp>
#include <engine/utils/AssetManager.hpp>

class Game;
class DebugInterface;

class Engine{
public:
    SDL_Window* window;
    Game* game;
    Input input;
    DebugInterface* debug;

    int screen_width;
    int screen_height;

    float time;
    float delta_time;
	

    Engine(int screen_width, int screen_height);
    ~Engine();

    int initialize(Game* game);

    void update(float delta_time);

    void render(float delta_time);


};


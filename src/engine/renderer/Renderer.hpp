#pragma once

#include <SDL_video.h>
#include "glm/glm.hpp"

#include "impl/GL.hpp"

#include <iostream>
#include <vector>

#include <engine/utils/FileLoader.hpp>
#include <engine/utils/AssetManager.hpp>

#include <engine/renderer/Shader.hpp>
#include <engine/renderer/Mesh.hpp>
#include <engine/renderer/Light.hpp>

class Camera;
class Entity;
class DebugInterface;
class Renderer{
public:
    Renderer();
    ~Renderer();

    void initialize(SDL_Window* window, int screen_width, int screen_height);

    void render(float delta_time);

    DebugInterface* debug;

    Camera* camera;

    vec4 ambient_light;

    //Shader
    Shader* shader;

    //OpenGL
    SDL_Window* window;
    SDL_GLContext glcontext;


    float time = 0;

    int max_shadow_maps = 1;
    int shadow_width = 3000;
    int shadow_height = 3000;

    int screen_width;
    int screen_height;

    static Renderer* instance;

private:

    void _render_entity(Entity* entity);
    void _render_pool(DynamicPool<Entity*> pool);
};

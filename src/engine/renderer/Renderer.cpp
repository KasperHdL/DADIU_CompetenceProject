#include "engine/renderer/Renderer.hpp"
#include "engine/Engine.hpp"
#include "debug/DebugInterface.hpp"
#include "engine/renderer/Camera.hpp"
#include "engine/Entity.hpp"

Renderer* Renderer::instance = nullptr;

Renderer::Renderer(){}

Renderer::~Renderer(){
    SDL_GL_DeleteContext(glcontext);
    instance = nullptr;
}

void Renderer::initialize(SDL_Window* window, int screen_width, int screen_height){
    if (instance != nullptr){
        std::cerr << "Multiple versions of Renderer initialized. Only a single instance is supported." << std::endl;
    }

    instance = this;

    this->window        = window;
    this->screen_width  = screen_width;
    this->screen_height = screen_height;

    glcontext = SDL_GL_CreateContext(window);

    glewExperimental = GL_FALSE;
    GLenum err = glewInit();


    if(err != GLEW_OK)
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //camera
    camera = new Camera();
    camera->set_viewport(0,0,screen_width, screen_height);
    camera->set_perspective_projection();

    ambient_light = vec4(0.2f);

    //Shader setup
    {
        shader = AssetManager::get_shader("shaders/standard");

        shader->init_uniform("model"        , Shader::Uniform_Type::Mat4);
        shader->init_uniform("view"         , Shader::Uniform_Type::Mat4);
        shader->init_uniform("projection"   , Shader::Uniform_Type::Mat4);
        shader->init_uniform("normalMat"    , Shader::Uniform_Type::Mat3);

        shader->init_uniform("ambientLight" , Shader::Uniform_Type::Vec4);
        shader->init_uniform("color"        , Shader::Uniform_Type::Vec4);
        shader->init_uniform("specularity"  , Shader::Uniform_Type::Float);


        for(int i = 0; i < 4; i++){
            shader->init_uniform("lightPosType[" + to_string(i) + "]", Shader::Uniform_Type::Vec4);
            shader->init_uniform("lightColorRange[" + to_string(i) + "]", Shader::Uniform_Type::Vec4);
        }

    }
}


void Renderer::render(float delta_time){


    glDepthMask(GL_TRUE);
    glClearColor(ambient_light.r, ambient_light.g, ambient_light.b, ambient_light.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    int w,h;
    SDL_GetWindowSize(window,&w,&h);

    time += delta_time;

    camera->set_viewport(0,0,w,h);
    camera->set_perspective_projection(); 



    ////////////////////
    // RENDER SCENE
    ///////////////////
    
    shader->use();

    //CUBE
    if(God::cube_mesh_entities.count > 0){
        Mesh::get_cube()->bind();

        for(int i = 0; i < God::cube_mesh_entities.capacity;i++){
            Entity** p = God::cube_mesh_entities[i];
            if(p != nullptr){
                Entity* e = *p;
                if(e->mesh != nullptr){
                    _render_entity(e);
                }
            }
        }
    }

    //SPHERE
    if(God::sphere_mesh_entities.count > 0){
        Mesh::get_sphere()->bind();

        for(int i = 0; i < God::sphere_mesh_entities.capacity;i++){
            Entity** p = God::sphere_mesh_entities[i];
            if(p != nullptr){
                Entity* e = *p;
                if(e->mesh != nullptr){
                    _render_entity(e);
                }
            }
        }
    }

    //QUADS
    if(God::quad_mesh_entities.count > 0){
        Mesh::get_quad()->bind();

        for(int i = 0; i < God::quad_mesh_entities.capacity;i++){
            Entity** p = God::quad_mesh_entities[i];
            if(p != nullptr){
                Entity* e = *p;
                if(e->mesh != nullptr){
                    _render_entity(e);
                }
            }
        }
    }

    //CUSTOM
    if(God::custom_mesh_entities.count > 0){
        for(int i = 0; i < God::custom_mesh_entities.capacity;i++){
            Entity** p = God::custom_mesh_entities[i];
            if(p != nullptr){
                Entity* e = *p;
                if(e->mesh != nullptr){
                    e->mesh->bind();
                    _render_entity(e);
                }
            }
        }
    }

    ///////////////////////////////
    //Debug UI
    ///////////////////////////////

    debug->render(delta_time);

    ///////////////////////////////
    //Swap Window
    ///////////////////////////////

    SDL_GL_SwapWindow(window);

    return;
}

void Renderer::_render_entity(Entity* entity){

    if(entity->transform == nullptr){
        cout << entity->name << "->transform = nullptr\n";

        return;
    }

    //vert
    shader->set_uniform("model"      , entity->transform->get_model_transform());
    shader->set_uniform("normalMat"  , entity->transform->get_normal_transform());

    shader->set_uniform("view"       , camera->view_transform);
    shader->set_uniform("projection" , camera->projection_transform);

    shader->set_uniform("ambientLight", ambient_light);
    shader->set_uniform("color", entity->color);

    shader->set_uniform("specularity", entity->specularity);

    //lights
    for(int i = 0; i < 4;i++){
        Light* l = God::lights[i];
        if(l != nullptr){

            vec4 light_pos_type = vec4(l->position, (int)l->type); 
            vec4 light_color_range = vec4(l->color * l->intensity, l->range);

            shader->set_uniform("lightPosType[" + to_string(i) + "]", light_pos_type);
            shader->set_uniform("lightColorRange[" + to_string(i) + "]", light_color_range);

        }
    }

    int indexCount = (int) entity->mesh->indices.size();
    if (indexCount == 0){
        glDrawArrays((GLenum)entity->mesh->topology, 0, entity->mesh->vertex_count);
    } else {
        glDrawElements((GLenum) entity->mesh->topology, indexCount, GL_UNSIGNED_SHORT, 0);
    }

}

void Renderer::_render_pool(DynamicPool<Entity*> pool){
    for(int i = 0; i < pool.capacity;i++){
        Entity* e = *pool[i];
        if(e != nullptr){
            cout << "i " << i << "\n";
            _render_entity(e);
        }
    }
}

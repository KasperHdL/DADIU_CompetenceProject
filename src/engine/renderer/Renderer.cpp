#include <engine/renderer/Renderer.hpp>
#include <engine/Engine.hpp>
#include <debug/DebugInterface.hpp>
#include <engine/renderer/Camera.hpp>

Renderer* Renderer::instance = nullptr;

Renderer::Renderer(){}

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

Renderer::~Renderer(){

    SDL_GL_DeleteContext(glcontext);
    instance = nullptr;
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

    mat4 camera_v = camera->view_transform;
    mat4 camera_p = camera->projection_transform;


    ////////////////////
    // RENDER SCENE
    ///////////////////
    
    for(int i = 0; i < God::entities.capacity;i++){
        Entity* e = God::entities[i];
        if(e != nullptr && e->mesh != nullptr){
            //set uniforms
            //Shader* shader = e->shader;
            shader->use();

            glm::mat4 t = glm::translate(mat4(), e->position);
            glm::mat4 s = glm::scale(mat4(), e->scale);
            glm::mat4 a = glm::eulerAngleYXZ(e->rotation.x, e->rotation.y, e->rotation.z);

            glm::mat4 model_transform = t * a * s;
            glm::mat3 normal_matrix = transpose(inverse((glm::mat3)model_transform));

            //vert
            shader->set_uniform("model"      , model_transform);
            shader->set_uniform("normalMat"  , normal_matrix);

            shader->set_uniform("view"       , camera_v);
            shader->set_uniform("projection" , camera_p);

            shader->set_uniform("ambientLight", ambient_light);
            shader->set_uniform("color", e->color);

            shader->set_uniform("specularity", e->specularity);

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

            //draw mesh
            e->mesh->bind();

            int indexCount = (int) e->mesh->indices.size();
            if (indexCount == 0){
                glDrawArrays((GLenum)e->mesh->topology, 0, e->mesh->vertex_count);
            } else {
                glDrawElements((GLenum) e->mesh->topology, indexCount, GL_UNSIGNED_SHORT, 0);
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

void Renderer::_render_scene(Shader* shader){


}


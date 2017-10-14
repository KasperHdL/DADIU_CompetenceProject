#pragma once

#include <glm/glm.hpp>

#include "SDL.h"
#include <string>

#include "engine/God.hpp"
#include "engine/renderer/Mesh.hpp"
#include "engine/renderer/Shader.hpp"

#include "engine/Transform.hpp"

//Debug
#include "engine/renderer/imgui/imgui_impl_sdl_gl3.hpp"
#include "engine/Input.hpp"

using namespace glm;

class Entity
{
public:
    //general
    bool debug_control = false;
    std::string name;

    Transform* transform = nullptr;

    Mesh* mesh = nullptr;

    Shader* shader;

    //@TODO to be removed
    vec4 color = vec4(1,1,1,1);
    float specularity = 50;


    Entity(){
        *God::entities.create() = this;
        transform = new (God::transforms.create()) Transform();
    }

    ~Entity(){
    }

    virtual void draw_debug_inspector(float dt, float control_speed){
        ImGui::Checkbox("Control", &debug_control);

        if(debug_control){
            if(Input::get_key_down(SDL_SCANCODE_W))
                transform->position.z += control_speed * dt;
             if(Input::get_key_down(SDL_SCANCODE_A))
                transform->position.x += control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_S))
                transform->position.z -= control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_D))
                transform->position.x -= control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_SPACE))
                transform->position.y += control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_LSHIFT))
                transform->position.y -= control_speed * dt;
        }

        ImGui::DragFloat3("Position", &transform->position.x, 0.1f);
        ImGui::DragFloat3("Scale",    &transform->scale.x,    0.1f);
        ImGui::DragFloat3("Rotation", &transform->rotation.x, 0.01f);

        ImGui::ColorEdit4("Color", &color.x);
        ImGui::DragFloat("Specularity", &specularity, 0.01f);

    }

    void set_mesh_as_cube(){
        mesh = Mesh::get_cube();

        *God::cube_mesh_entities.create() = this;
    }

    void set_mesh_as_sphere(){
        mesh = Mesh::get_sphere();

        *God::sphere_mesh_entities.create() = this;
    }

    void set_mesh_as_quad(){
        mesh = Mesh::get_quad();

        *God::quad_mesh_entities.create() = this;
    }

    void set_mesh_as_custom(Mesh* mesh){
        this->mesh = mesh;

        *God::custom_mesh_entities.create() = this;
    }


};

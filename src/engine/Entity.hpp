#pragma once

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "SDL.h"
#include <string>

#include <engine/renderer/Mesh.hpp>
#include <engine/renderer/Shader.hpp>

//Debug
#include <engine/renderer/imgui/imgui_impl_sdl_gl3.hpp>
#include <engine/Input.hpp>

using namespace glm;

class Entity
{
public:
    //general
    bool debug_control = false;
    std::string name;

    //mesh
    Mesh* mesh = nullptr;

    //Transform
    vec3 position;
    vec3 scale;
    vec3 rotation;

    //Shader
    Shader* shader;

    //@TODO to be removed
    vec4 color = vec4(1,1,1,1);
    float specularity = 50;


    Entity(){
    }

    ~Entity(){

    }

    void draw_debug_inspector(float dt, float control_speed){
        if(ImGui::TreeNode(name.c_str())){
            ImGui::Checkbox("Control", &debug_control);
            if(debug_control){
                if(Input::get_key_down(SDL_SCANCODE_W))
                    position.z += control_speed * dt;
                 if(Input::get_key_down(SDL_SCANCODE_A))
                    position.x += control_speed * dt;
                if(Input::get_key_down(SDL_SCANCODE_S))
                    position.z -= control_speed * dt;
                if(Input::get_key_down(SDL_SCANCODE_D))
                    position.x -= control_speed * dt;
                if(Input::get_key_down(SDL_SCANCODE_SPACE))
                    position.y += control_speed * dt;
                if(Input::get_key_down(SDL_SCANCODE_LSHIFT))
                    position.y -= control_speed * dt;
            }

            ImGui::DragFloat3("Position", &position.x, 0.1f);
            ImGui::DragFloat3("Scale",    &scale.x,    0.1f);
            ImGui::DragFloat3("Rotation", &rotation.x, 0.01f);

            ImGui::DragFloat4("Color", &color.x, 0.01f);
            ImGui::DragFloat("Specularity", &specularity, 0.01f);

            ImGui::TreePop();
        }
    }


};

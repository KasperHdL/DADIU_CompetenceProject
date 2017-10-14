#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "engine/renderer/Renderer.hpp"
#include "engine/renderer/Camera.hpp"

#include "engine/Entity.hpp"
#include "engine/God.hpp"

 

class DebugCamera : public Entity{ 
    public: 
        Camera* camera;

        float speed = 15;
        float rotation_speed = .3f;
        float rot_max_x = (((float)60/180) * glm::pi<float>());

        bool freeze = true;
 
        DebugCamera(){ 
            name = "Debug Camera";

            transform->position = vec3(10,10,10);

            camera = Renderer::instance->camera;

        } 

        void update(float dt){
            if(Input::get_key_on_down(SDL_SCANCODE_F))
                freeze = !freeze;
            if(freeze){
                return;
            }


            //position
            vec3 move_direction = vec3(0,0,0);

            if(Input::get_key_down(SDL_SCANCODE_W))
                move_direction += vec3(0,0,-1);
            if(Input::get_key_down(SDL_SCANCODE_A))
                move_direction += vec3(-1,0,0);
            if(Input::get_key_down(SDL_SCANCODE_S))
                move_direction += vec3(0,0,1);
            if(Input::get_key_down(SDL_SCANCODE_D))
                move_direction += vec3(1,0,0);
            if(Input::get_key_down(SDL_SCANCODE_SPACE))
                move_direction += vec3(0,1,0);
            if(Input::get_key_down(SDL_SCANCODE_LSHIFT))
                move_direction += vec3(0,-1,0);

            if(move_direction != vec3(0,0,0))
                transform->position += mat3(eulerAngleY(-transform->rotation.y)) * (normalize(move_direction) * speed * dt);

            //rotation
            vec2 delta_mouse = Input::get_mouse_delta();

            transform->rotation.x += delta_mouse.y * rotation_speed * dt;
            transform->rotation.x = clamp<float>(transform->rotation.x, -rot_max_x, rot_max_x);
            transform->rotation.y += delta_mouse.x * rotation_speed * dt;

            //update camera
            camera->view_transform = (eulerAngleX(transform->rotation.x) * eulerAngleY(transform->rotation.y)) * glm::translate(mat4(), -transform->position);
            camera->transform->position = transform->position;
            camera->transform->rotation = transform->rotation;


        }
 
        ~DebugCamera(){ 
 
        } 
 
}; 

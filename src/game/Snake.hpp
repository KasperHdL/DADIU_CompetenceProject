#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <engine/renderer/Renderer.hpp>
#include <engine/renderer/Camera.hpp>

#include <engine/Entity.hpp>
#include <engine/God.hpp>

#include <engine/renderer/Mesh.hpp>

 

class Snake{ 
    public: 
        Entity* entity; 

        float speed = 15;
 
        Snake(){ 
            entity = new (God::entities.create()) Entity();
            entity->name = "Snake";

            entity->position = vec3(10,10,10);

            entity->mesh = Mesh::get_cube();

        } 

        void update(float dt){
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
                entity->position += normalize(move_direction) * speed * dt;


        }
 
        ~Snake(){ 
 
        } 
 
}; 

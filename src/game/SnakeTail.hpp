#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <engine/Entity.hpp>
#include <engine/God.hpp>

#include <engine/renderer/Mesh.hpp>

 

class SnakeTail{ 
    public: 
        Entity* entity; 

        SnakeTail(int index){ 
            entity = new (God::entities.create()) Entity();
            entity->name = "SnakeTail";

            entity->position = vec3(0,0,0);

            entity->mesh = Mesh::get_cube();
            entity->scale = vec3(0.5f);
            entity->color = vec4(0,1,0,1);
            entity->specularity = 10;

        } 

        void set_position(vec3 pos){
            entity->position = pos;

        }
 
        ~SnakeTail(){ 
 
        } 
 
}; 

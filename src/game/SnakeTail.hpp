#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <engine/Entity.hpp>
#include <engine/Transform.hpp>

#include <engine/renderer/Mesh.hpp>

 

class SnakeTail : public Entity{ 
    public: 

        SnakeTail(int index){ 
            name = "SnakeTail";

            transform->position = vec3(0);
            transform->scale = vec3(0.5f);

            set_mesh_as_cube();

            color = vec4(0,1,0,1);
            specularity = 10;

        } 

        void set_position(vec3 pos){
            transform->position = pos;

        }
 
        ~SnakeTail(){ 
 
        } 
 
}; 

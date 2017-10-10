#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <engine/Entity.hpp>
#include <engine/God.hpp>

#include <engine/renderer/Mesh.hpp>

 

class Fruit{ 
    public: 
        Entity* entity; 

        float speed = 15;
 
        Fruit(){ 
            entity = new (God::entities.create()) Entity();
            entity->name = "Fruit";

            entity->position = vec3(10,10,10);

            entity->mesh = Mesh::get_cube();

        } 

        void update(float dt){

        }
 
        ~Fruit(){ 
 
        } 
 
}; 

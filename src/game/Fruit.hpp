#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>


#include <engine/Entity.hpp>
#include <engine/God.hpp>

#include <engine/renderer/Mesh.hpp>

 

class Fruit{ 
    public: 
        Entity* entity; 

        float start_delay = 5;

        bool is_active;

        float time;
        float delay;

        int playarea;

        Fruit(int playarea){ 
            this->playarea = playarea;

            entity = new (God::entities.create()) Entity();
            entity->name = "Fruit";

            entity->position = vec3(10,10,10);
            entity->scale = vec3(.5f);

            entity->mesh = Mesh::get_sphere();
            entity->color = vec4(1,0,0,0);
            entity->specularity = 10;
        } 

        void restart(){
            is_active = false;
            entity->position = vec3(10,10,10);
            
            delay = start_delay;
            time = 0;
        }

        void update(float delta_time){
            if(is_active) return;

            time += delta_time;

            if(time >= delay){
                is_active = true;
                
                int x = (std::rand() % (playarea * 2)) - playarea;
                int y = (std::rand() % (playarea * 2)) - playarea;

                entity->position = vec3(x, 0, y);
            }
        }

        void collected(){
            is_active = false;
            time = 0;
            entity->position = vec3(10,10,10);

            delay *= 0.95f;
        }
 
        ~Fruit(){ 
 
        } 
 
}; 

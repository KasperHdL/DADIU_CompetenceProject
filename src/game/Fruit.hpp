#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

#include <engine/Entity.hpp>

class Fruit : public Entity{ 
    public: 

        float start_delay = 5;

        bool is_active;

        float time;
        float delay;

        int playarea;

        Fruit(int playarea){ 
            this->playarea = playarea;

            name = "Fruit";

            transform->position = vec3(10);
            transform->scale = vec3(.5f);

            set_mesh_as_sphere();

            color = vec4(1,0,0,0);
            specularity = 10;
        } 

        void restart(){
            is_active = false;
            transform->position = vec3(10);
            
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

                transform->position = vec3(x, 0, y);
            }
        }

        void collected(){
            is_active = false;
            time = 0;
            transform->position = vec3(10);

            delay *= 0.95f;
        }
 
        ~Fruit(){ 
 
        } 
 
}; 

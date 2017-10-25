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

        vec2 playarea;

        Fruit(vec2 playarea){ 
            this->playarea = playarea / 2.0f;

            name = "Fruit";

            transform->position = vec3(10);
            transform->scale = vec3(.1f);

            set_mesh_as_sphere();

			is_visible = false;
            color = vec4(1,.5f,0,0);
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
				is_visible = true;

				float x = glm::linearRand<float>(-playarea.x, playarea.x);
				float y = glm::linearRand<float>(-playarea.y, playarea.y);

                transform->position = vec3(x, 1, y);
            }
        }

        void collected(){
            is_active = false;
			is_visible = false;
            time = 0;
            transform->position = vec3(10);

            delay *= 0.95f;
        }
 
        ~Fruit(){ 
 
        } 
 
}; 

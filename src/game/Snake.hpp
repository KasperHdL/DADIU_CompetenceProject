#pragma once 

#include <iostream>
#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <engine/Entity.hpp>
#include <engine/Transform.hpp>

#include <game/SnakeTail.hpp>

 

class Snake : public Entity{ 
    public: 

        DynamicPool<SnakeTail> tail = DynamicPool<SnakeTail>(8);


        //start values

        float start_move_delay = 0.01f;
        vec3 origin = vec3(0,0,0);

        //values
        float move_timer;
        float move_delay;

		float move_length = 0.5f;


        vec3 last_movement;
        vec3 input;

        int playarea;
        vec3 local_pos;


        int snake_length;

        bool is_dead;

 
        Snake(int playarea){ 
            this->playarea = playarea;

            name = "Snake";

            transform->position = origin;
            transform->scale = vec3(.1f);

            set_mesh_as_sphere();

            color = vec4(1,0,0,1);
            specularity = 10;

            snake_length = 3;

            for(int i = 0; i < snake_length; i++){
                SnakeTail* t = new (tail.create()) SnakeTail(i);
            }

        } 

        void restart(){
            is_dead = false;

            move_delay = start_move_delay;
            move_timer = -move_delay;

            snake_length = 3;
            local_pos = vec3(0);
            transform->position = origin;

            input = vec3(1,0,0);
			last_movement = input;

            for(int i = 0; i < tail.capacity;i++){
                SnakeTail* t = tail[i];
                if(t != nullptr){
                    if(i < snake_length)
                        t->transform->position = origin;
                    else
                        t->transform->position = vec3(10,10,10);
                }
            }
        }

        void update(float dt){
            if(is_dead){
                return;
            }

			move_timer += dt;

			if (move_timer >= move_delay) {
				move_timer -= move_delay;

				mat4 controller_matrix = Input::controller_matrix[0];
				vec3 controller = vec3(controller_matrix * vec4(0, 0, 0, 1));

				input = controller - local_pos;

				move_tail();

				input = normalize(input) * move_length;


				local_pos += input * dt;
				transform->position = origin + local_pos;
			}

           // is_dead = collision_check();

        }

        bool collision_check(){

            //check collision with walls
            if(glm::abs(local_pos.x) > playarea || glm::abs(local_pos.y) > playarea){
                return true;
            }

            //check with tail
            for(int i = 0; i < snake_length;i++){
                SnakeTail* t = tail[i];
                if(t != nullptr){

                    if(transform->position == t->transform->position){
                        return true;
                    }
                }
            }

            return false;
        }

        void move_tail(){
            vec3 pos_a = transform->position;
            vec3 pos_b = pos_a;

            for(int i = 0; i < snake_length;i++){
                SnakeTail* t = tail[i];
                if(t != nullptr){
                    pos_a = t->transform->position;
                    t->transform->position = pos_b;
                    pos_b = pos_a;
                }
            }
        }


        void fruit_collected(){
            snake_length++;

            if(tail.count < snake_length){
                SnakeTail* t = new (tail.create()) SnakeTail(snake_length);
                t->transform->position = transform->position;
            }else{
                SnakeTail* t = tail[snake_length-1];
                t->transform->position = transform->position;

            }

            move_delay *= 0.9f;
        }

        void draw_debug_inspector(float dt, float control_speed){
            Entity::draw_debug_inspector(dt, control_speed);

            ImGui::Text("Snake Settings:");
            ImGui::Separator();

            ImGui::DragFloat("Start Move Delay", &start_move_delay, 0.005f);
            ImGui::DragFloat("Current Move Delay", &move_delay, 0.005f);

        }

 
        ~Snake(){ 
 
        } 
 
}; 

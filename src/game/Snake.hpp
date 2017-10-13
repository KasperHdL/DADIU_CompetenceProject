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

        float start_move_delay = 0.3f;
        vec3 origin = vec3(0,0,0);

        //values
        float move_timer;
        float move_delay;


        vec2 last_movement;
        vec2 last_legal_input;
        vec2 input;

        int playarea;
        vec2 local_pos;


        int snake_length;

        bool is_dead;

 
        Snake(int playarea){ 
            this->playarea = playarea;

            name = "Snake";

            transform->position = origin;
            transform->scale = vec3(.5f);

            set_mesh_as_cube();

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
            local_pos = vec2(0);
            transform->position = origin;

            last_movement = vec2(0,0);
            input = vec2(1,0);
            last_legal_input = input;

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


            if(Input::get_key_down(SDL_SCANCODE_UP))
                input = vec2(0,-1);
            else if(Input::get_key_down(SDL_SCANCODE_LEFT))
                input = vec2(-1,0);
            else if(Input::get_key_down(SDL_SCANCODE_RIGHT))
                input = vec2(1,0);
            else if(Input::get_key_down(SDL_SCANCODE_DOWN))
                input = vec2(0,1);

            if(-input == last_movement){
                input = last_legal_input;
            }else{
                last_legal_input = input;
            }

            move_timer += dt;

            if(move_timer >= move_delay){
                move_timer -= move_delay;

                move_tail();

                if(-input == last_movement)input = last_movement;
                local_pos += input;
                last_movement = input;
                transform->position = origin + vec3(local_pos.x, 0 ,local_pos.y);

                is_dead = collision_check();

            }
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

 
        ~Snake(){ 
 
        } 
 
}; 

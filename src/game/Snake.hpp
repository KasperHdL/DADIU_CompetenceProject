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

        DynamicPool<SnakeTail> tail = DynamicPool<SnakeTail>(16);


        //start values

        float start_move_delay = 0.01f;
        vec3 origin = vec3(0,0,0);

        //values
        float move_timer;
        float move_delay;

		float move_length = 0.5f;


        vec3 last_movement;
        vec3 input;

        vec2 playarea;
        vec3 local_pos;


        int snake_length;

        bool is_dead;

 
        Snake(vec2 playarea){ 
            this->playarea = playarea;


            name = "Snake";

            transform->scale = vec3(.1f);

            set_mesh_as_sphere();

            color = vec4(1,0,0,1);

        } 

        void restart(){
            is_dead = false;

            move_delay = start_move_delay;
            move_timer = -move_delay;

            local_pos = vec3(0);
            transform->position = origin;

            input = vec3(1,0,0);
			last_movement = input;

			SnakeTail* last;
			snake_length = 0;
			int initial_snake_length = 5;
			for (int i = 0; i < tail.capacity; i++) {
				if (i < initial_snake_length){
					create_tail_piece();
				}else {
					SnakeTail* t = tail[i];
					if (t != nullptr) {
						t->is_visible = false;
					}
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

				vec3 controller = vec3(Input::get_vr_controller_matrix(0)[3]);

				if (glm::length(controller) != 0) {

					input = controller - local_pos;

					update_tail(dt);

					input = normalize(input) * move_length;

					local_pos += input * dt;
				}

				transform->position = origin + local_pos;
			}

           is_dead = collision_check();
		  // cout << " == " << (is_dead ? "true" : "false") << "\n";

        }

        bool collision_check(){

			//cout << "lp [" << local_pos.x << ", " << local_pos.y << ", " << local_pos.z << "]";
			//cout << "\tps [" << playarea.x << ", " << playarea.y << "]";

            //check collision with walls
            if(glm::abs(local_pos.x) > playarea.x / 2.0f || glm::abs(local_pos.z) > playarea.y / 2.0f){
                return true;
            }

            //check with tail
            for(int i = 2; i < snake_length;i++){
                SnakeTail* t = tail[i];
                if(t != nullptr){
                    if(check_collision(transform->position, transform->scale.x, t->transform->position, 0.0f)){
                        return true;
                    }
                }
            }
			

            return false;
        }

        void update_tail(float dt){
            for(int i = 0; i < snake_length;i++){
                SnakeTail* t = tail[i];
                if(t != nullptr){
					t->update(dt);
                }
            }
        }


        void fruit_collected(){
			create_tail_piece();

            move_delay *= 0.9f;
        }

		void create_tail_piece() {
			snake_length++;

			SnakeTail* t;

			if (tail.count < snake_length) {
				t = new (tail.create()) SnakeTail(snake_length);
			}
			else {
				t = tail[snake_length - 1];
			}

			t->transform->position = vec3(-10);
			t->is_visible = true;

			if(snake_length-2 == -1)
				tail[snake_length - 1]->linked_transform = transform;
			else
				tail[snake_length - 1]->linked_transform = tail[snake_length - 2]->transform;

		}

		inline bool check_collision(vec3 p1, float d1, vec3 p2, float d2) {
			return length(p1 - p2) < d1 + d2;
		}


		void draw_debug_inspector(float dt, float control_speed) {
			Entity::draw_debug_inspector(dt, control_speed);

			ImGui::Text("Snake Settings:");
			ImGui::Separator();

			ImGui::DragFloat("Start Move Delay", &start_move_delay, 0.005f);
			ImGui::DragFloat("Current Move Delay", &move_delay, 0.005f);

		}

 
        ~Snake(){ 
 
        } 
 
}; 

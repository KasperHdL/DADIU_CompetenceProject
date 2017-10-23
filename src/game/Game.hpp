#pragma once

#include <string>
#include <vector>

#include <engine/renderer/Mesh.hpp>

#include <engine/Engine.hpp>
#include <engine/God.hpp>
#include <engine/utils/DynamicPool.hpp>

#include <game/DebugCamera.hpp>
#include <game/Snake.hpp>
#include <game/Fruit.hpp>


class Game{
    public:
        Engine* engine;
       // DebugCamera* debug_camera;

        Snake* snake;
        Fruit* fruit;

        DynamicPool<Entity> walls = DynamicPool<Entity>(6);

        float restart_timer = 0;
        float restart_delay = 1;

        int fruits_collected = 0;
        int max_fruits_collected = 0;


        Game(){}
        ~Game(){}

        void initialize(Engine* engine){
            this->engine = engine;

            //debug_camera = new DebugCamera();
            //debug_camera->transform->position = vec3(1.1,6.4,12.5);
            //debug_camera->transform->rotation = vec3(0.55,6.25,0);
			//debug_camera->update_camera();

            float playarea = 1;

            snake = new Snake(playarea);
            fruit = new Fruit(playarea);

            Light* sun =  new (God::lights.create()) Light(Light::Type::Directional, vec3(0,-1,.25f), vec3(1,1,1), .15f);

			Entity* e = new (walls.create()) Entity();

			e->name = "Wall";

			e->transform->position = vec3(0, 0, 0);
			e->transform->scale = vec3(2);
			e->transform->rotation = vec3(0, glm::half_pi<float>(), 0);

			e->set_mesh_as_quad();

			e->color = vec4(1);

            restart();
        }

        void restart(){
            fruit->restart();
            snake->restart();

            restart_timer = 0;
            fruits_collected = 0;
        }

        void update(float delta_time){
            //debug_camera->update(delta_time);

            if(snake->is_dead){
                restart_timer += delta_time;

                if(restart_timer >= restart_delay){
                    restart();
                }

                return;
            }


            snake->update(delta_time);
            fruit->update(delta_time);

            if(fruit->is_active){
                //check if fruit and snake is at the same place

                if(check_collision(snake->transform, fruit->transform)){
                    fruits_collected++;

                    if(fruits_collected > max_fruits_collected) 
                        max_fruits_collected = fruits_collected;

                    snake->fruit_collected();
                    fruit->collected();

                }

            }

        }

		inline bool check_collision(Transform* t1, Transform* t2) {
			return length(t1->position - t2->position) < t1->scale.x + t2->scale.x;
		}

		inline bool check_collision(vec3 p1, float d1, vec3 p2, float d2) {
			return length(p1 - p2) < d1 + d2;
		}

        void draw_debug(){
            ImGui::Begin("Game");
            ImGui::Text("Fruits Collected: %i\tMax: %i", fruits_collected, max_fruits_collected);
            ImGui::End();
        }

       
};

#pragma once

#include <string>
#include <vector>

#include "engine/renderer/Mesh.hpp"

#include "engine/Engine.hpp"
#include "engine/God.hpp"
#include "engine/utils/DynamicPool.hpp"

#include "game/DebugCamera.hpp"
#include "game/Snake.hpp"
#include "game/Fruit.hpp"


class Game{
    public:
        Engine* engine;
        DebugCamera* debug_camera;

        Snake* snake;
        Fruit* fruit;

        DynamicPool<Entity> walls = DynamicPool<Entity>(32);

        float restart_timer = 0;
        float restart_delay = 1;

        int fruits_collected = 0;
        int max_fruits_collected = 0;


        Game(){}
        ~Game(){}

        void initialize(Engine* engine){
            this->engine = engine;

            debug_camera = new DebugCamera();
            debug_camera->transform->position = vec3(1.1,6.4,12.5);
            debug_camera->transform->rotation = vec3(0.55,6.25,0);

            int playarea = 5;

            snake = new Snake(playarea);
            fruit = new Fruit(playarea);

            Light* sun =  new (God::lights.create()) Light(Light::Type::Directional, vec3(0,-1,.25f), vec3(1,1,1), .15f);

            create_walls(playarea);

            restart();
        }

        void restart(){
            fruit->restart();
            snake->restart();

            restart_timer = 0;
            fruits_collected = 0;
        }

        void update(float delta_time){
            debug_camera->update(delta_time);

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

                if(snake->transform->position == fruit->transform->position){
                    fruits_collected++;

                    if(fruits_collected > max_fruits_collected) 
                        max_fruits_collected = fruits_collected;

                    snake->fruit_collected();
                    fruit->collected();

                }

            }

        }

        void draw_debug(){
            ImGui::Begin("Game");
            ImGui::Text("Fruits Collected: %i\tMax: %i", fruits_collected, max_fruits_collected);
            ImGui::End();
        }

        void create_walls(int playarea){
            Entity* e;
            playarea += 1;

            vec4 color = vec4(0,0,1,1);

            for(int x = -playarea; x <= playarea; x ++){
                for(int y = -playarea; y <= playarea; y ++){
                    if(x == -playarea || x == playarea || y == -playarea || y == playarea){
                        e = new (walls.create()) Entity();

                        e->name = "Wall [" + std::to_string(x) + ", " + std::to_string(y) + "]";

                        e->transform->position = vec3(x, 0, y);
                        e->transform->scale = vec3(0.5f, 0.5f, 0.5f);

                        e->set_mesh_as_cube();

                        e->color = color;

                    }
                }
            }
        }
};

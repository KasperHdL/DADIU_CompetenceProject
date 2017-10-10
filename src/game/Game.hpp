#pragma once

#include <string>
#include <vector>

#include <engine/Engine.hpp>
#include <engine/utils/SceneLoader.hpp>
#include <engine/God.hpp>
#include <game/DebugCamera.hpp>


class Game{
    public:
        Engine* engine;
        DebugCamera* debug_camera;
        Snake* snake;

        Game(){}
        ~Game(){}

        void initialize(Engine* engine){
            this->engine = engine;

            debug_camera = new DebugCamera();
            debug_camera->entity->position = vec3(-3.3,-1.3,11.3);
            debug_camera->entity->rotation = vec3(0.15,6.5,0);

            snake = new Snake();

            SceneLoader::load_scene("standard.scene");
            //hack to remove spec from landscape
            God::entities[God::entities.count-1]->specularity = 0;

            Light* sun =  new (God::lights.create()) Light(Light::Type::Directional, vec3(0,-1,.25f), vec3(1,1,1), .15f);
            sun->set_ortho_scale(15);
            sun->create_shadow_map = true;

        }

        void update(float delta_time){

            debug_camera->update(delta_time);

        }

        void draw_debug(){



        }

};

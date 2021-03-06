#pragma once

#include "SDL.h"
#include <string>

#include <game/Game.hpp>

#include <engine/utils/Timer.hpp>
#include <engine/Input.hpp>

#include <engine/God.hpp>

#include <engine/renderer/imgui/imgui_impl_sdl_gl3.hpp>
#include <engine/renderer/Mesh.hpp>
#include <engine/renderer/Light.hpp>
#include <engine/renderer/Camera.hpp>

class DebugInterface{
    public:

        Game* game;
        SDL_Window* window;

        bool enabled = true;
        int n;
		
		bool use_camera = false;
		Camera* camera;

        //profile plots
        static const int array_length = 500;
        int array_index = 0;

            //delta
            float delta_time_plot[array_length] = {};
            float delta_time_sum = 0;

            //update
            float update_plot[array_length] = {};
            float update_sum = 0;
            Timer update_timer;
            
        //menu
        bool menu = true;
        bool hierarchy = false;
        bool scene_manager = false;
        bool create = false;
        bool hotload_shader = true;
        bool profiler = false;

        //windows

            //scene manager
            char filename[50];

            //create menu
            int type_selected_index = 0;
            static const int type_num_items = 4;
            const char* type_entities[type_num_items] = {"Cube", "Quad", "Sphere", "Light"};

            //hierarchy
            float control_speed = 15;
            bool draw_light_pos = false;


        void initialize(SDL_Window* window, Game* game){
            ImGui_ImplSdlGL3_Init(window);

            this->game = game;
            this->window = window;

			camera = new Camera();
        }

        void update(float dt){
            if(Input::get_key_on_down(SDL_SCANCODE_ESCAPE)){ 
                enabled       = !enabled;
                if(enabled && !menu && !hierarchy && scene_manager && !create) menu = true;
            }

			if(Input::get_key_on_down(SDL_SCANCODE_F))       use_camera		= !use_camera;

            if(Input::get_key_on_down(SDL_SCANCODE_F2 ))     menu           = !menu;
            if(Input::get_key_on_down(SDL_SCANCODE_F3 ))     hierarchy      = !hierarchy;
            if(Input::get_key_on_down(SDL_SCANCODE_F4 ))     scene_manager  = !scene_manager;
            if(Input::get_key_on_down(SDL_SCANCODE_F5 ))     create         = !create;
            if(Input::get_key_on_down(SDL_SCANCODE_F6 ))     profiler       = !profiler;
            if(Input::get_key_on_down(SDL_SCANCODE_F10))     hotload_shader = !hotload_shader;

        }

        void render(float dt){

            if(enabled){
                int id = 0;

                ImGui_ImplSdlGL3_NewFrame(window);

                game->draw_debug();

                //menu
                if(menu){
                    ImGui::Begin("Menu");
                        ImGui::Text("Move on [WASD], [Space] and [Shift]");
                        ImGui::Checkbox("Debug Camera [F]", &use_camera);
                        ImGui::Separator();

                        ImGui::Checkbox("Debug         [Esc]", &enabled);
                        ImGui::Checkbox("Menu          [F2] ", &menu);
                        ImGui::Checkbox("Hierarchy     [F3] ", &hierarchy);
                        ImGui::Checkbox("Scene Manager [F4] ", &scene_manager);
                        ImGui::Checkbox("Create Menu   [F5] ", &create);
                        ImGui::Checkbox("Profiler      [F6] ", &profiler);

                        ImGui::Checkbox("Hotload Shader[F10] ", &hotload_shader);
						
                        ImGui::Separator();

                    ImGui::End();
                }

                if(profiler){
                    ImGui::Begin("Profiler");
                        //PLOTS
                        
                        update_plots(dt);

                        ImGui::Separator();

                        ImGui::Text("Delta time in ms");
                        ImGui::PlotLines("", delta_time_plot, array_length);
                        ImGui::Text("%3.3f \t- Avg: %3.3f",1000 * dt, (1000 * delta_time_sum) / (double)array_length);


                        ImGui::Text("Update in ms");
                        ImGui::PlotLines("", update_plot, array_length);
                        ImGui::Text("%3.3f \t- Avg: %3.3f",1000 * update_timer.duration, (1000 * update_sum) / (double)array_length);

                        array_index = ++array_index % array_length;

                    ImGui::End();

                }

                //windows
                if(hierarchy){
                    ImGui::Begin("Hierarchy");
                    ImGui::DragFloat("Control Speed", &control_speed);
                    ImGui::Checkbox("Show Light Pos", &draw_light_pos);
                    ImGui::Separator();

                    //light

                    ImGui::Text("");
                    ImGui::Text("Lights:");
                    ImGui::Separator();
                    for(int i = 0; i < God::lights.capacity;i++){
                        Light* l = God::lights[i];
                        if(l != nullptr){
                            ImGui::PushID(id++);
                            l->draw_debug_inspector(dt, control_speed);
                            ImGui::PopID();
                            ImGui::Separator();
                        }
                    }

                    ImGui::Text("");
                    ImGui::Text("Entities:");
                    ImGui::Separator();
                    //entities
                    for(int i = 0; i < God::entities.capacity;i++){
                        Entity** p = God::entities[i];
                        if(p == nullptr) continue;

                        Entity* e = *p;
                        if(e != nullptr){
                            ImGui::PushID(id++);

                            if(ImGui::TreeNode(e->name.c_str())){
                                e->draw_debug_inspector(dt, control_speed);

                                ImGui::TreePop();
                            }

                            ImGui::PopID();
                            ImGui::Separator();
                        }
                    }

                    ImGui::End();
                }
                
                if(create){
                    ImGui::Begin("Create Menu");
                    ImGui::Combo("Type", &type_selected_index, type_entities, type_num_items);
                    if(ImGui::Button("Create")){
                        _create_entity(type_selected_index);
                    }


                    ImGui::End();
                }

                ImGui::Render();
            }
        }


        void _create_entity(int index){
            if(index == 3){
                //Light
                Light* l = new (God::lights.create()) Light();

            }else{
                /*
                Entity* e = new (God::entities.create()) Entity();

                e->name = type_entities[type_selected_index];
                if(e->name == "Cube")
                    e->mesh = Mesh::get_cube();
                else if(e->name == "Quad")
                    e->mesh = Mesh::get_quad();
                else if(e->name == "Sphere")
                    e->mesh = Mesh::get_sphere();

                e->position = glm::vec3();
                e->scale = glm::vec3(1,1,1);
                e->rotation = glm::vec3();
                */
            }
        }

        void update_plots(float dt){
            
            delta_time_sum -= delta_time_plot[array_index];
            delta_time_plot[array_index] = dt;
            delta_time_sum += delta_time_plot[array_index];

            update_sum -= update_plot[array_index];
            update_plot[array_index] = update_timer.duration;
            update_sum += update_plot[array_index];
        }

};

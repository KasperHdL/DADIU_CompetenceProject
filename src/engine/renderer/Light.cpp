#include <engine/renderer/Light.hpp>


Light::Light(){
    this->type = Light::Type::Point;
    type_selected_index = (int)type;
    this->position = vec3(0);
    this->color = vec3(1);
    this->intensity = 1;
    this->range = 10;
}

Light::Light(Light::Type type, glm::vec3 position, glm::vec3 color, float intensity){
    //[Point or Directional]
    this->type = type;
    type_selected_index = (int)type;

    this->position  = position;
    this->direction = glm::vec3(0,0,1);
    this->color     = color;
    this->intensity = intensity;

    range = 10;
    falloff = 10;

}

Light::Light(Light::Type type, glm::vec3 position, glm::vec3 direction, float falloff, glm::vec3 color, float intensity){
    //[Spot]
    this->type = type;
    type_selected_index = (int)type;

    this->position = position;
    this->direction = direction;
    this->color = color;
    this->intensity = intensity;
    this->falloff = falloff;

    range = 10;
}

void Light::draw_debug_inspector(float dt, float control_speed){
    if(ImGui::TreeNode(type_names[type_selected_index])){
        ImGui::Checkbox("Control", &_debug_control);

        if(_debug_control){
            if(Input::get_key_down(SDL_SCANCODE_W))
                position.z += control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_A))
                position.x += control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_S))
                position.z -= control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_D))
                position.x -= control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_SPACE))
                position.y += control_speed * dt;
            if(Input::get_key_down(SDL_SCANCODE_LSHIFT))
                position.y -= control_speed * dt;
        }

        ImGui::Combo("Type", &type_selected_index, type_names, type_name_length);
        if((Light::Type)type_selected_index != type){
            type = (Light::Type)type_selected_index;
        }

        if(type >= Type::Point)
            ImGui::DragFloat3("Position"    , &position.x    , 0.1f);

        if(type == Type::Directional)
            ImGui::DragFloat3("Direction"   , &position.x    , 0.1f);

        if(type == Type::Spot)
            ImGui::DragFloat3("Direction"   , &direction.x   , 0.1f);

        ImGui::ColorEdit3("Color"           , &color.r);
        ImGui::DragFloat("Intensity"        , &intensity     , 0.1f);

        if(type >= Type::Point)
            ImGui::DragFloat("Range" , &range, 0.1f);

        if(type == Type::Spot){
            ImGui::DragFloat("Fall Off"      , &falloff        , 0.01f);
        }

        ImGui::TreePop();

    }
}

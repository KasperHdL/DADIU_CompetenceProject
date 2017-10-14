#pragma once 
#include "glm/glm.hpp"

#include "imgui/imgui_impl_sdl_gl3.hpp"

#include <string>

#include <engine/renderer/Mesh.hpp>

#include <engine/Input.hpp>

class Light{
private:
    bool _debug_control = false;
    int type_selected_index;
    static const int type_name_length = 3;
    const char* type_names[type_name_length] = {"Directional Light", "Point Light", "Spot Light"};

public:
    enum class Type{
        Directional,
        Point,
        Spot
    };

    Type type;
    glm::vec3 position;
    glm::vec3 color;

    //[point / spot]
    float intensity;
    float range;

    //[spot]
    float falloff;
    glm::vec3 direction;

    Light();

    Light(Type type, glm::vec3 position, glm::vec3 color, float intensity);

    Light(Type type, glm::vec3 position, glm::vec3 direction, float falloff, glm::vec3 color, float intensity);

    void draw_debug_inspector(float dt, float control_speed);
};

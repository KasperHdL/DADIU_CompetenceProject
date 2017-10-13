#pragma once

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "SDL.h"
#include <string>

using namespace glm;

class Transform
{
public:
    vec3 position;
    vec3 scale;
    vec3 rotation;

    Transform(){
        position = vec3(0);
        scale = vec3(1);
        rotation = vec3(0);
    }

    ~Transform(){

    }


    mat4 get_model_transform(){
        glm::mat4 t = glm::translate(mat4(), position);
        glm::mat4 s = glm::scale(mat4(), scale);
        glm::mat4 a = glm::eulerAngleYXZ(rotation.x, rotation.y, rotation.z);

        return t * a * s;
    }

    mat3 get_normal_transform(){
        return transpose(inverse((glm::mat3)get_model_transform()));
    }
};

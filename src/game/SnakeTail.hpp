#pragma once 

#include <string> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <engine/Entity.hpp>
#include <engine/Transform.hpp>

#include <engine/renderer/Mesh.hpp>

 

class SnakeTail : public Entity{ 
    public: 

		Transform* linked_transform;
		
		float distance = 0.1f;

        SnakeTail(int index){ 
            name = "SnakeTail";

            transform->position = vec3(0);
            transform->scale = vec3(0.1f);

			set_mesh_as_sphere();

            color = vec4(0,1,0,1);

        } 

		inline void update(float dt) {
			vec3 delta = transform->position - linked_transform->position;

			float l = length<float>(delta);

			if (l > distance) {
				transform->position = linked_transform->position + normalize(delta) * distance;
			}
		}
 
        ~SnakeTail(){ 
 
        } 
 
}; 

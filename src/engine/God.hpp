#pragma once

#include <engine/utils/DynamicPool.hpp>
#include <engine/utils/Pool.hpp>

class Transform;
class Light;
class Camera;
class Entity;

class God{
    public:

    static DynamicPool<Light> lights;

    //entity
    static DynamicPool<Transform> transforms;

//////////
//POINTERS

    static DynamicPool<Entity*> entities;
    
    //meshes
    static DynamicPool<Entity*> cube_mesh_entities;
    static DynamicPool<Entity*> sphere_mesh_entities;
    static DynamicPool<Entity*> quad_mesh_entities;

    static DynamicPool<Entity*> custom_mesh_entities;


};

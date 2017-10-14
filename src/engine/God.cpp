#include "engine/God.hpp"

#include "engine/Transform.hpp"
#include "engine/Entity.hpp"
#include "engine/renderer/Light.hpp"
#include "engine/renderer/Camera.hpp"

DynamicPool<Transform> God::transforms         = DynamicPool<Transform>(32);

DynamicPool<Light> God::lights                 = DynamicPool<Light>(10);

//References
DynamicPool<Entity*> God::entities             = DynamicPool<Entity*>(64);

//Meshes
DynamicPool<Entity*> God::cube_mesh_entities   = DynamicPool<Entity*>(8);
DynamicPool<Entity*> God::sphere_mesh_entities = DynamicPool<Entity*>(8);
DynamicPool<Entity*> God::quad_mesh_entities   = DynamicPool<Entity*>(8);

DynamicPool<Entity*> God::custom_mesh_entities = DynamicPool<Entity*>(8);


#include <engine/God.hpp>

#include <engine/Entity.hpp>
#include <engine/renderer/Light.hpp>
#include <engine/renderer/Camera.hpp>

DynamicPool<Entity> God::entities = DynamicPool<Entity>(32);
DynamicPool<Light> God::lights = DynamicPool<Light>(10);

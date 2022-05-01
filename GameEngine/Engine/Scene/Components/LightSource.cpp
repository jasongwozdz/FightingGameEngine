#include "LightSource.h"
#include "Transform.h"
#include "../../Renderer/VkTypes.h"
#include "../../Renderer/VkRenderer.h"

LightSource::LightSource(Entity* entity) :
	entity_(entity)
{
}

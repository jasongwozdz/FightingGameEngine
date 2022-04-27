#include "LightSource.h"
#include "Transform.h"

LightSource::LightSource(Entity* entity) :
	entity_(entity),
	color_({1.0f}),
	diffuse_({1.0f}),
	ambient_({1.0f}),
	specular_({1.0f})
{}

void LightSource::uploadLightSourceData()
{
	Transform* transform = entity_->tryGetComponent<Transform>();
	_ASSERT_EXPR(transform, ("Can't upload light source data since there is no transform"));

}

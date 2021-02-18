#include "Renderable.h"

Renderable::Renderable(std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName) :
	vertices_(vertices), indices_(indices), depthEnabled_(depthEnabled), entityName_(entityName)
{
	uploaded_ = false;
	delete_ = false;
	depthEnabled_ = true;
	draw_ = true;
}

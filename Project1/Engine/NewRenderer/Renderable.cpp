#include "Renderable.h"

Renderable::Renderable(std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName) :
	vertices_(vertices), indices_(indices), depthEnabled_(depthEnabled), entityName_(entityName)
{
	uploaded_ = false;
	delete_ = false;
	depthEnabled_ = true;
	draw_ = true;
	ubo_ = new Ubo;
}

Renderable::Renderable(Renderable&& other) :
	vertices_(other.vertices_),
	indices_(other.indices_),
	entityName_(other.entityName_),
	uploaded_(other.uploaded_),
	delete_(other.delete_),
	depthEnabled_(other.depthEnabled_),
	draw_(other.draw_)
{
	ubo_ = other.ubo_;
	other.ubo_ = nullptr;
}

Renderable::Renderable(const Renderable& other) :
	vertices_(other.vertices_),
	indices_(other.indices_),
	entityName_(other.entityName_),
	uploaded_(other.uploaded_),
	delete_(other.delete_),
	depthEnabled_(other.depthEnabled_),
	draw_(other.draw_)
{
	ubo_ = new Ubo;
	memcpy(ubo_, &other.ubo_, sizeof(other.ubo_));
}

Renderable::~Renderable()
{
	delete ubo_;
}

Ubo& Renderable::ubo()
{
	return *ubo_;
}

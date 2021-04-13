#include "Renderable.h"

Renderable::Renderable()
{
	std::cout << "JASON: Constructor2" << std::endl;
	ubo_ = new Ubo;
	textureResources_ = new TextureResources;
}

Renderable::Renderable(std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName) :
	vertices_(vertices), indices_(indices), depthEnabled_(depthEnabled), entityName_(entityName)
{
	std::cout << "JASON: constructer 2" << std::endl;
	uploaded_ = false;
	delete_ = false;
	depthEnabled_ = true;
	draw_ = true;
	ubo_ = new Ubo;
	textureResources_ = new TextureResources;
}

Renderable::Renderable(Renderable&& other) :
	vertices_(other.vertices_),
	indices_(other.indices_),
	entityName_(other.entityName_),
	uploaded_(other.uploaded_),
	delete_(other.delete_),
	depthEnabled_(other.depthEnabled_),
	draw_(other.draw_),
	vertexBuffer_(other.vertexBuffer_),
	vertexMem_(other.vertexMem_),
	indexBuffer_(other.indexBuffer_),
	indexMem_(other.indexMem_),
	uniformBuffer_(other.uniformBuffer_),
	uniformMem_(other.uniformMem_),
	pipelineType_(other.pipelineType_),
	renderableObjectId_(other.renderableObjectId_),
	descriptorLayout_(other.descriptorLayout_),
	descriptorSets_(other.descriptorSets_),
	allocator_(other.allocator_),
	logicalDevice_(other.logicalDevice_)
{
	std::cout << "JASON: moved" << std::endl;
	other.vertexBuffer_ = nullptr;
	other.vertexMem_ = nullptr;
	other.indexBuffer_ = nullptr;
	other.vertexBuffer_ = nullptr;
	for (int i = 0; i < other.uniformBuffer_.size(); i++)
	{
		other.uniformBuffer_[i] = nullptr;
		other.uniformMem_[i] = nullptr;
	}
	other.descriptorLayout_ = nullptr;
	for (int i = 0; i < other.descriptorSets_.size(); i++)
	{
		other.descriptorSets_[i] = nullptr;
	}
	textureResources_ = other.textureResources_;
	other.textureResources_ = nullptr;
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
	draw_(other.draw_),
	vertexBuffer_(other.vertexBuffer_),
	vertexMem_(other.vertexMem_),
	indexBuffer_(other.indexBuffer_),
	indexMem_(other.indexMem_),
	uniformBuffer_(other.uniformBuffer_),
	uniformMem_(other.uniformMem_),
	pipelineType_(other.pipelineType_),
	renderableObjectId_(other.renderableObjectId_),
	descriptorLayout_(other.descriptorLayout_),
	descriptorSets_(other.descriptorSets_),
	allocator_(other.allocator_),
	logicalDevice_(other.logicalDevice_)
{
	std::cout << "Jason copied" << std::endl;
	textureResources_ = new TextureResources;
	memcpy(textureResources_, other.textureResources_, sizeof(TextureResources));
	ubo_ = new Ubo;
	memcpy(ubo_, other.ubo_, sizeof(Ubo));
}

Renderable::~Renderable()
{
	if (uploaded_)
	{
		deleteResources(allocator_, logicalDevice_);
	}
	delete ubo_;
	delete textureResources_;
}

Ubo& Renderable::ubo()
{
	return *ubo_;
}

void Renderable::deleteResources(VmaAllocator& allocator, VkDevice& logicalDevice)
{
	std::cout << "Jason delete" << std::endl;
	vmaDestroyBuffer(allocator, vertexBuffer_, vertexMem_);
	vmaDestroyBuffer(allocator, indexBuffer_, indexMem_);
	for (int i = 0; i < uniformBuffer_.size(); i++)
	{
		vmaDestroyBuffer(allocator, uniformBuffer_[i], uniformMem_[i]);
	}
	//vkDestroyDescriptorSetLayout(logicalDevice, descriptorLayout_, nullptr);
}

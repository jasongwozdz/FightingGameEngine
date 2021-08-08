#include "Renderable.h"

Renderable::Renderable(std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool depthEnabled, std::string entityName) :
	vertices_(vertices), indices_(indices), depthEnabled_(depthEnabled), entityName_(entityName)
{
	uploaded_ = false;
	delete_ = false;
	depthEnabled_ = true;
}

Renderable::Renderable(Renderable&& other) :
	vertices_(other.vertices_),
	indices_(other.indices_),
	entityName_(other.entityName_),
	uploaded_(other.uploaded_),
	delete_(other.delete_),
	depthEnabled_(other.depthEnabled_),
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
	logicalDevice_(other.logicalDevice_),
	isLine_(other.isLine_)
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
	ubo_ = other.ubo_;
}

Renderable::Renderable(const Renderable& other) :
	vertices_(other.vertices_),
	indices_(other.indices_),
	entityName_(other.entityName_),
	uploaded_(other.uploaded_),
	delete_(other.delete_),
	depthEnabled_(other.depthEnabled_),
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
	logicalDevice_(other.logicalDevice_),
	isLine_(other.isLine_)
{
	std::cout << "Jason copied" << std::endl;
	textureResources_ = other.textureResources_;
	ubo_ = other.ubo_;
}

Renderable::~Renderable()
{
	if (uploaded_)
	{
		deleteResources(allocator_, logicalDevice_);
	}
}

void Renderable::deleteResources(VmaAllocator& allocator, VkDevice& logicalDevice)
{
	std::cout << "Jason delete mesh" << std::endl;
	vmaDestroyBuffer(allocator, vertexBuffer_, vertexMem_);
	vmaDestroyBuffer(allocator, indexBuffer_, indexMem_);
	for (int i = 0; i < uniformBuffer_.size(); i++)
	{
		vmaDestroyBuffer(allocator, uniformBuffer_[i], uniformMem_[i]);
	}
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorLayout_, nullptr);
}

void Renderable::setVertAndIndicies(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	assert(allocator_);
	vertices_ = vertices;
	indices_ = indices;
	size_t vertexBufferSize = vertices_.size() * sizeof(Vertex);
	void *vertexData;
	vmaMapMemory(allocator_, vertexMem_, &vertexData);
	memcpy(vertexData, vertices_.data(), vertexBufferSize);
	vmaUnmapMemory(allocator_, vertexMem_);

	size_t indexBufferSize = indices_.size() * sizeof(uint32_t);
	void *indexData;
	vmaMapMemory(allocator_, indexMem_, &indexData);
	memcpy(indexData, indices_.data(), indexBufferSize);
	vmaUnmapMemory(allocator_, indexMem_);
}

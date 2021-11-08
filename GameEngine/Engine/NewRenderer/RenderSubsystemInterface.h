#pragma once
#include <vulkan/vulkan.h>
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"

class RenderSubsystemInterface
{
public:
	RenderSubsystemInterface(VkDevice& logicalDevice, VkRenderPass& renderPass, VmaAllocator& allocator, VkDescriptorPool& descriptorPool) :
		logicalDevice_(logicalDevice),
		renderPass_(renderPass),
		allocator_(allocator),
		descriptorPool_(descriptorPool)
	{}//In derived classes initalize pipelines, descriptorSets, textures, etc...

	virtual ~RenderSubsystemInterface() = default;

	virtual void renderFrame(VkCommandBuffer commandBuffer, uint32_t currentSwapChainIndex) = 0; //called by main renderer every frame
protected:
	VkDevice& logicalDevice_;
	VkRenderPass& renderPass_;
	VmaAllocator& allocator_;
	VkDescriptorPool& descriptorPool_;
};

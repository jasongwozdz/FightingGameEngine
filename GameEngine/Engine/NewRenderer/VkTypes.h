#pragma once
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"

struct VulkanBuffer
{
	VkBuffer buffer_;
	VmaAllocation mem_;
};

struct VulkanImage
{
	VkImage image_;
	VmaAllocation mem_;
};

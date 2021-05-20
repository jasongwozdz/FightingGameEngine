#pragma once
#include <vulkan/vulkan.h>
#include <Window.h>
#include "glm/glm.hpp"
#include "PipelineBuilder.h"
#include "libs/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "Singleton.h"
#include <array>

namespace UI
{
	template<typename TypeX, typename TypeY>
	struct ScrollingBuffer
	{
		std::vector<TypeX> bufferX;
		std::vector<TypeY> bufferY;
		int bufferSize;
		int offset = 0;

		void add(TypeX x, TypeY y)
		{
			if (bufferX.size() < bufferSize)
			{
				bufferX.push_back(x);
				bufferY.push_back(y);
			}
			else
			{
				bufferX[offset] = x;
				bufferY[offset] = y;
				offset = (offset + 1) % bufferSize;
			}
		}
	};

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec2 aUV;
		glm::vec4 color;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, aUV);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};

	struct PushConstantInfo
	{
		alignas(8)glm::vec2 scale;
		alignas(8)glm::vec2 pos;
	};

	struct RenderInfo
	{
		PushConstantInfo pushConstantInfo;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indicies;
	};


	class UIInterface : Singleton<UIInterface>
	{
	public:
		static UIInterface& getSingleton();
		static UIInterface* getSingletonPtr();

		UIInterface(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, GLFWwindow* window, VkRenderPass& renderPass, VmaAllocator& allocator);

		void recreateUI(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, GLFWwindow* window, VkRenderPass& renderPass);

		void drawRect(int width, int height, glm::vec2 pos, glm::vec4 color);

		~UIInterface();

		void prepareFrame();

		void renderFrame(VkCommandBuffer& curentBuffer);

		std::vector<UI::RenderInfo> drawData_;
		std::vector<VkDeviceSize> vertexOffsets_;
		std::vector<VkDeviceSize> indexOffsets_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indicies_;
		VkDeviceSize globalVertexOffset_ = 0;
		VkDeviceSize globalIndexOffset_ = 0;

		PipelineBuilder::PipelineResources* uiPipeline_;

		VkDescriptorSetLayout descriptorLayout_;

		VmaAllocator& allocator_;

		VkBuffer vertexBuffer_;
		VkBuffer indexBuffer_;
		VmaAllocation vertexBufferMem_;
		VmaAllocation indexBufferMem_;

	};
}

#pragma once
#include <vulkan/vulkan.h>
#include <Window.h>

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
}

class UIInterface
{
public:
	UIInterface(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, GLFWwindow* window, VkRenderPass& renderPass);

	~UIInterface();

	void prepareFrame();

	void renderFrame(VkCommandBuffer& curentBuffer);
};

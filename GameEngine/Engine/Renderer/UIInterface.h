#pragma once
#include <vulkan/vulkan.h>
#include "../Window.h"
#include "glm/glm.hpp"
#include "PipelineBuilder.h"
#include "../libs/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "../Singleton.h"
#include <array>
#include "../Vertex.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

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


	struct PushConstantInfo
	{
		alignas(8)glm::vec2 scale;
		alignas(8)glm::vec2 pos;
	};

	struct RenderInfo
	{
		PushConstantInfo pushConstantInfo;
		std::vector<UIVertex> vertices;
		std::vector<uint32_t> indicies;
	};


	class ENGINE_API UIInterface : Singleton<UIInterface>
	{
	public:
		static UIInterface& getSingleton();
		static UIInterface* getSingletonPtr();

		UIInterface(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, GLFWwindow* window, VkRenderPass& renderPass, VmaAllocator& allocator);
		~UIInterface();
		void recreateUI(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& logicalDevice, uint32_t queueFamily, VkQueue& queue, VkDescriptorPool& descriptorPool, int minImageCount, int imageCount, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer, GLFWwindow* window, VkRenderPass& renderPass);

		void prepareFrame();
		void renderFrame(VkCommandBuffer& curentBuffer);

		//Custom UI elements
		void drawRect(int width, int height, glm::vec2 pos, glm::vec4 color);

		//IMGUI interface
		void showImGuiDemoWindow();
		bool beginMenu(const std::string& menuName);
		bool addMenuItem(const std::string& menuItem, bool* clickedOn);
		void endMenu();
		void beginWindow(const std::string& windowTitle, float width, float height, glm::vec2 pos, bool* isOpen = NULL, bool isTransparent = false);
		void EndWindow();
		void addText(const std::string& text);
		bool addInput(const std::string& defaulText, std::string* input);
		void openPopup(const std::string& popupName);
		void closePopup();
		bool beginPopup(const std::string& popupName, bool* isOpen);
		void endPopup();
		bool addButton(const std::string& buttonText);
		void makeColumns(const std::string& tableName, int numCols);
		void addSeperator();
		void nextColumn();
		void sameLine(float offset, float spacing);
		bool addSelectable(const std::string& label, bool selected);
		bool addSelectable(const std::string& label, bool selected, float sizeX, float sizeY);
		bool beginPopupContextItem(const std::string& name);
		bool beginMainMenuBar();
		void endMainMenuBar();
		bool addInputFloat(std::string text, float& output, float defaultVal);
		bool addInputInt(std::string text, int& output, int step);
		bool addSlider(const std::string& text, int& input, int start, int end);
		void addTextToTransparentBackground(const std::string& text, glm::vec2 pos, const glm::vec4& color, float scale);
		void drawGrid(std::vector<std::pair<std::string, glm::vec4>> elements, std::vector<bool> selected, int rows, int cols, int width, int height);
		bool isMouseOverUI();
		void centerNextWindow();
		glm::vec2 getCursorPos();//Get cursor position using Imgui api

	private:
		std::vector<UI::RenderInfo> drawData_;
		std::vector<VkDeviceSize> vertexOffsets_;
		std::vector<VkDeviceSize> indexOffsets_;
		std::vector<UIVertex> vertices_;
		std::vector<uint32_t> indicies_;
		VkDescriptorPool descriptorPool_;
		VkDeviceSize globalVertexOffset_ = 0;
		VkDeviceSize globalIndexOffset_ = 0;
		PipelineResources* uiPipeline_;
		VkDescriptorSetLayout descriptorLayout_;
		VmaAllocator& allocator_;
		VkBuffer vertexBuffer_;
		VkBuffer indexBuffer_;
		VmaAllocation vertexBufferMem_;
		VmaAllocation indexBufferMem_;
		char imGuiInputBuffer_[256];
	};
}

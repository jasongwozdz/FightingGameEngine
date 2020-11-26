#include "UserInterfaceImp.h"
#include "Renderer.h"
#include "BufferOperations.h"

UserInterfaceImp::UserInterfaceImp(GLFWwindow* window)
{
	ImGui::CreateContext();

	Renderer& renderer = Renderer::getSingleton();
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = renderer.getVulkanInstance();
    init_info.PhysicalDevice = renderer.getPhysicalDevice();
    init_info.Device = renderer.getLogicalDevice();
    init_info.QueueFamily = renderer.getQueueFamily();
    init_info.Queue = renderer.getGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = renderer.getDescriptorPool();
    init_info.Allocator = NULL;
	init_info.MinImageCount = renderer.getSwapChainImages().size();
    init_info.ImageCount = renderer.getSwapChainImages().size();
	init_info.CheckVkResultFn = VK_NULL_HANDLE;

	m_depthBufferComponenet = new DepthBufferComponent(renderer.getLogicalDevice(), renderer.getPhysicalDevice(), renderer.getCommandPool(), VK_FORMAT_D32_SFLOAT);

	m_renderPass = new RenderPassComponent(renderer.getLogicalDevice());

    ImGui_ImplVulkan_Init(&init_info, renderer.getRenderPass().m_renderPass);

	{
		VkCommandPool commandPool = renderer.getCommandPool();
		VkCommandBuffer commandBuffer = renderer.getCurrentCommandBuffer();
		auto err = vkResetCommandPool(renderer.getLogicalDevice(), commandPool, 0);
		VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &begin_info);

		ImGui_ImplVulkan_CreateFontsTexture(renderer.getCurrentCommandBuffer());

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &commandBuffer;
        err = vkEndCommandBuffer(commandBuffer);
        err = vkQueueSubmit(renderer.getGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE);

		vkDeviceWaitIdle(renderer.getLogicalDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

UserInterfaceImp::~UserInterfaceImp()
{
	m_renderPass->~RenderPassComponent();
	delete m_renderPass;
}

void UserInterfaceImp::prepareFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();	
	ImGui::ShowDemoWindow();
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

void UserInterfaceImp::renderFrame(VkCommandBuffer& currentBuffer)
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, currentBuffer);
}

#include "Renderer.h"
#include "Window.h"

int main(int argc, char** argv)
{
	Renderer renderer = Renderer();
	Window* window = renderer.InitWindow(640, 480, "Vulkan Test");

	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = renderer.GetVulkanGraphicsQueueFamilyIndex();
	vkCreateCommandPool(renderer.GetVulkanDevice(), &commandPoolCreateInfo, VK_NULL_HANDLE, &commandPool);

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(renderer.GetVulkanDevice(), &commandBufferAllocateInfo, &commandBuffer);

	while (renderer.Run())
	{
		window->BeginRender();

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		ErrorCheck(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		VkRect2D renderArea{};
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		renderArea.extent = window->GetVulkanSurfaceSize();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].depthStencil.depth = 0.0f;
		clearValues[0].depthStencil.stencil = 0;
		clearValues[1].color.float32[0] = 0.0f;
		clearValues[1].color.float32[1] = 0.0f;
		clearValues[1].color.float32[2] = 0.0f;
		clearValues[1].color.float32[3] = 0.0f;

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//renderPassBeginInfo.pNext;
		renderPassBeginInfo.renderPass = window->GetVulkanRenderPass();
		renderPassBeginInfo.framebuffer = window->GetVulkanActiveFramebuffer();
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(commandBuffer);
		ErrorCheck(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
		submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

		ErrorCheck(vkQueueSubmit(renderer.GetVulkanQueue(), 1, &submitInfo, VK_NULL_HANDLE));

		ErrorCheck(vkQueueWaitIdle(renderer.GetVulkanQueue()));
		window->EndRender();
	}

	vkDestroyCommandPool(renderer.GetVulkanDevice(), commandPool, nullptr);

	return 0;
}
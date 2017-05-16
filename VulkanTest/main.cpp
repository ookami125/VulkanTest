#include "Renderer.h"
#include "Window.h"
#include "Scene.h"
#include "VulkanTools.h"
#include "Mesh.h"
#include "SO_DynamicMesh.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
int main(int argc, char** argv)
{
	Renderer renderer = Renderer();
	Window* window = renderer.InitWindow(640, 480, "Vulkan Test");
	Scene scene(&renderer);

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

	VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	//semaphoreCreateInfo.pNext;
	//semaphoreCreateInfo.flags;
	vkCreateSemaphore(renderer.GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);

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
		clearValues[1].color.float32[0] = 0.0f; //red
		clearValues[1].color.float32[1] = 0.0f; //green
		clearValues[1].color.float32[2] = 0.5f; //blue
		clearValues[1].color.float32[3] = 1.0f; //alpha

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
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

		ErrorCheck(vkQueueSubmit(renderer.GetVulkanQueue(), 1, &submitInfo, VK_NULL_HANDLE));

		ErrorCheck(vkQueueWaitIdle(renderer.GetVulkanQueue()));
		window->EndRender({renderCompleteSemaphore});
		if (GetAsyncKeyState(VK_ESCAPE) & 0x1)
			break;
	}

	vkQueueWaitIdle(renderer.GetVulkanQueue());

	vkDestroySemaphore(renderer.GetVulkanDevice(), renderCompleteSemaphore, nullptr);

	vkDestroyCommandPool(renderer.GetVulkanDevice(), commandPool, nullptr);

	return 0;
}
*/

constexpr uint32_t TRIANGLE_COUNT = 20;

int main()
{
	std::vector<std::string> pipeline_names{
		"default"
	};
	Renderer renderer(pipeline_names);
	Window		*	window = renderer.InitWindow( 800, 600, "test");
	Scene		*	scene = new Scene(&renderer);

	// mesh triangle. This is data only.
	Mesh triangle;

	std::vector<SO_DynamicMesh*> sobj(TRIANGLE_COUNT);		// list of dynamic meshes
	std::vector<float> sobj_rot_diff(sobj.size());			// rotational difference between animations on meshes.
	for (uint32_t i = 0; i < sobj.size(); ++i) {
		SO_DynamicMesh* so = new SO_DynamicMesh(nullptr, &renderer, &triangle);
		sobj[i]->SetActiveWindow(window);								// set active window, trying to get rid of this step
		sobj[i]->SetActivePipeline(window->GetPipelines()[0]);		// set active pipeline, this step is required but there might be a lot nicer way of doing it
		scene->AddObject(so);
		sobj_rot_diff[i] = float(i * 3.141592653589f * 2 * 0.01);			// last value is in "circles", 1.0f equals one full round per object.
	}

	float rotator = 0.0f;		// simple ever increasing float

	while (renderer.Run()) {
		rotator += 0.0015f;		// increasing the "float counter". This just moves the vertices around a little

								// update meshes manually, ideally this would be it's own entity with a link to a scene_object.
		for (uint32_t i = 0; i < sobj.size(); ++i) {
			sobj[i]->GetEditableVertices()[0].loc[0] = cos(rotator + sobj_rot_diff[i]) / 2.0f;	// x
			sobj[i]->GetEditableVertices()[0].loc[1] = sin(rotator + sobj_rot_diff[i]) / 2.0f;	// y
		}
		scene->Update();					// update scene, this handles all general stuff, including vertex uploads to GPU, this is recursive
		window->RenderScene(scene, true);		// render scene, this is also recursive
	}
	vkQueueWaitIdle(renderer.GetVulkanQueue());
	//system("pause");
	return 0;
}
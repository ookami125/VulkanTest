#include "Window.h"
#include "Renderer.h"
#include "Shared.h"
#include "Window_win32.cpp"
#include "Window_xcb.cpp"

Window::Window(Renderer* _renderer, uint32_t width, uint32_t height, char* name)
{
	m_Renderer = _renderer;
	m_Width = width;
	m_Height = height;
	m_WindowName = name;
	InitOSWindow();
	InitSurface();
	InitSwapchain();
	InitSwapchainImages();
	InitDepthStencilImage();
	InitRenderPass();
	InitFramebuffers();
	InitSyncronizations();
}

Window::~Window()
{
	DestroySyncronizations();
	DestroyFramebuffers();
	DestroyRenderPass();
	DestroyDepthStencilImage();
	DestroySwapchainImages();
	DestroySwapchain();
	DestroySurface();
	DestroyOSWindow();
}

void Window::Close()
{
	m_ShouldClose = true;
}

bool Window::Update()
{
	UpdateOSWindow();
	return !m_ShouldClose;
}

void Window::BeginRender()
{
	//TODO: vkAcquireNextImageKHR tries to grab more images then it's suppose to.
	ErrorCheck(vkAcquireNextImageKHR(m_Renderer->GetVulkanDevice(), m_Swapchain, UINT64_MAX, VK_NULL_HANDLE, m_SwapchainImageAvailable, &m_ActiveSwapchainImageId));
	ErrorCheck(vkWaitForFences(m_Renderer->GetVulkanDevice(), 1, &m_SwapchainImageAvailable, VK_TRUE, UINT64_MAX));
	ErrorCheck(vkResetFences(m_Renderer->GetVulkanDevice(), 1, &m_SwapchainImageAvailable));
	ErrorCheck(vkQueueWaitIdle(m_Renderer->GetVulkanQueue()));
}

void Window::EndRender()
{
	VkResult presentResult = VkResult::VK_RESULT_MAX_ENUM;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	//presentInfo.pNext;
	presentInfo.waitSemaphoreCount = 0;
	presentInfo.pWaitSemaphores = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &m_ActiveSwapchainImageId;
	presentInfo.pResults = &presentResult;

	ErrorCheck(vkQueuePresentKHR(m_Renderer->GetVulkanQueue(), &presentInfo));
	ErrorCheck(presentResult);
}

VkRenderPass Window::GetVulkanRenderPass()
{
	return m_RenderPass;
}

VkFramebuffer Window::GetVulkanActiveFramebuffer()
{
	if(m_ActiveSwapchainImageId>=m_SwapchainImageAvailable)
		return m_Framebuffers[0];
	return m_Framebuffers[m_ActiveSwapchainImageId];
}

VkExtent2D Window::GetVulkanSurfaceSize()
{
	return{m_Width, m_Height};
}

void Window::InitSurface()
{
	InitOSSurface();

	auto gpu = m_Renderer->GetVulkanPhysicalDevice();

	VkBool32 WSI_Supported = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpu, m_Renderer->GetVulkanGraphicsQueueFamilyIndex(), m_Surface, &WSI_Supported);
	if (!WSI_Supported)
	{
		assert(0 && "WSI not supported");
		exit(-1);
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, m_Surface, &m_SurfaceCapabilities);
	if (m_SurfaceCapabilities.currentExtent.width < UINT32_MAX)
	{
		m_Width = m_SurfaceCapabilities.currentExtent.width;
		m_Height = m_SurfaceCapabilities.currentExtent.height;
	}

	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_Renderer->GetVulkanPhysicalDevice(), m_Surface, &formatCount, nullptr);
		if (formatCount == 0)
		{
			assert(0 && "Error: Surface format missing");
			exit(-1);
		}
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_Renderer->GetVulkanPhysicalDevice(), m_Surface, &formatCount, formats.data());

		/*printf("Surface Formats: \n");
		for (VkSurfaceFormatKHR format : formats)
		{
			printf("%d %d\n", format.format, format.colorSpace);
		}
		*/

		if (formats[0].format == VK_FORMAT_UNDEFINED)
		{
			m_SurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
			m_SurfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		}
		else
		{
			m_SurfaceFormat = formats[0];
		}
	}
}

void Window::DestroySurface()
{
	vkDestroySurfaceKHR(m_Renderer->GetVulkanInstance(), m_Surface, nullptr);
}

void Window::InitSwapchain()
{
	if (m_SurfaceCapabilities.maxImageCount > 0 && m_SwapchainImageCount > m_SurfaceCapabilities.maxImageCount) m_SwapchainImageCount = m_SurfaceCapabilities.maxImageCount;
	if (m_SwapchainImageCount < m_SurfaceCapabilities.minImageCount+1) m_SwapchainImageCount = m_SurfaceCapabilities.minImageCount+1;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t presentCount = 0;
		ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Renderer->GetVulkanPhysicalDevice(), m_Surface, &presentCount, VK_NULL_HANDLE));
		std::vector<VkPresentModeKHR> presentModes(presentCount);
		ErrorCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Renderer->GetVulkanPhysicalDevice(), m_Surface, &presentCount, presentModes.data()));
		for (VkPresentModeKHR pm : presentModes)
		{
			if (pm == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = pm;
		}
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	//swapchainCreateInfo.pNext					= ;
	//swapchainCreateInfo.flags					= ;
	swapchainCreateInfo.surface					= m_Surface;
	swapchainCreateInfo.minImageCount			= m_SwapchainImageCount;
	swapchainCreateInfo.imageFormat				= m_SurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace			= m_SurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent.width		= m_Width;
	swapchainCreateInfo.imageExtent.height		= m_Height;
	swapchainCreateInfo.imageArrayLayers		= 1;
	swapchainCreateInfo.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount	= 0;
	swapchainCreateInfo.pQueueFamilyIndices		= VK_NULL_HANDLE;
	swapchainCreateInfo.preTransform			= VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode				= presentMode;
	swapchainCreateInfo.clipped					= VK_TRUE;
	swapchainCreateInfo.oldSwapchain			= VK_NULL_HANDLE;
	ErrorCheck(vkCreateSwapchainKHR(m_Renderer->GetVulkanDevice(), &swapchainCreateInfo, nullptr, &m_Swapchain));

	ErrorCheck(vkGetSwapchainImagesKHR(m_Renderer->GetVulkanDevice(), m_Swapchain, &m_SwapchainImageCount, nullptr));

}

void Window::DestroySwapchain()
{
	vkDestroySwapchainKHR(m_Renderer->GetVulkanDevice(), m_Swapchain, nullptr);
}

void Window::InitSwapchainImages()
{
	m_Images.resize(m_SwapchainImageCount);
	m_ImageViews.resize(m_SwapchainImageCount);
	ErrorCheck(vkGetSwapchainImagesKHR(m_Renderer->GetVulkanDevice(), m_Swapchain, &m_SwapchainImageCount, m_Images.data()));
	for (uint32_t i = 0; i < m_SwapchainImageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//imageViewCreateInfo.flags;
		imageViewCreateInfo.image = m_Images[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_SurfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;

		ErrorCheck(vkCreateImageView(m_Renderer->GetVulkanDevice(), &imageViewCreateInfo, nullptr, &m_ImageViews[i]));
	}
}

void Window::DestroySwapchainImages()
{
	for (VkImageView imageView : m_ImageViews)
		vkDestroyImageView(m_Renderer->GetVulkanDevice(), imageView, nullptr);
}

void Window::InitDepthStencilImage()
{
	{
		std::vector<VkFormat> tryFormats{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM,
		};

		for (auto f : tryFormats)
		{
			VkFormatProperties formatProperties{};
			vkGetPhysicalDeviceFormatProperties(m_Renderer->GetVulkanPhysicalDevice(), f, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_DepthStencilFormat = f;
				break;
			}
		}
		if (m_DepthStencilFormat == VK_FORMAT_UNDEFINED)
		{
			assert(0 && "Error: Depth stencil format not selected");
			exit(-1);
		}
		if (m_DepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			m_DepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT||
			m_DepthStencilFormat == VK_FORMAT_D16_UNORM ||
			m_DepthStencilFormat == VK_FORMAT_S8_UINT)
		{
			m_DepthStencilAvailable = true;
		}
	}
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	//imageCreateInfo.pNext;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = m_DepthStencilFormat;
	imageCreateInfo.extent.width = m_Width;
	imageCreateInfo.extent.height = m_Height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	//imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//imageCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	//imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
	//imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	ErrorCheck(vkCreateImage(m_Renderer->GetVulkanDevice(), &imageCreateInfo, nullptr, &m_DepthStencilImage));

	VkMemoryRequirements imageMemoryRequirments{};
	vkGetImageMemoryRequirements(m_Renderer->GetVulkanDevice(), m_DepthStencilImage, &imageMemoryRequirments);

	uint32_t memoryIndex = FindMemoryTypeIndex(&m_Renderer->GetVulkanPhysicalDeviceMemoryProperties(), &imageMemoryRequirments, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	//memoryAllocateInfo.pNext;
	memoryAllocateInfo.allocationSize = imageMemoryRequirments.size;
	memoryAllocateInfo.memoryTypeIndex = memoryIndex;

	vkAllocateMemory(m_Renderer->GetVulkanDevice(), &memoryAllocateInfo, nullptr, &m_DepthStencilMemory);
	vkBindImageMemory(m_Renderer->GetVulkanDevice(), m_DepthStencilImage, m_DepthStencilMemory, 0);

	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	//imageViewCreateInfo.flags = ;
	imageViewCreateInfo.image = m_DepthStencilImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = m_DepthStencilFormat;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (m_DepthStencilAvailable?VK_IMAGE_ASPECT_STENCIL_BIT:0);
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	vkCreateImageView(m_Renderer->GetVulkanDevice(), &imageViewCreateInfo, nullptr, &m_DepthStencilImageView);
}

void Window::DestroyDepthStencilImage()
{
	vkDestroyImageView(m_Renderer->GetVulkanDevice(), m_DepthStencilImageView, nullptr);
	vkFreeMemory(m_Renderer->GetVulkanDevice(), m_DepthStencilMemory, nullptr);
	vkDestroyImage(m_Renderer->GetVulkanDevice(), m_DepthStencilImage, nullptr);
}

void Window::InitRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachments {};
	attachments[0].flags = 0;
	attachments[0].format = m_DepthStencilFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments[1].flags = 0;
	attachments[1].format = m_SurfaceFormat.format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//attachments[1].stencilLoadOp;		//stencil operations dont exist for the color buffer
	//attachments[1].stencilStoreOp;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference subPass0DepthStencilAttachmentReference{};
	subPass0DepthStencilAttachmentReference.attachment = 0;
	subPass0DepthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 1> subPass0ColorAttachmentReferences{};
	subPass0ColorAttachmentReferences[0].attachment = 1;
	subPass0ColorAttachmentReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkSubpassDescription, 1> subPasses {};
	//subPasses[0].flags;
	subPasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//subPasses[0].inputAttachmentCount = 0;
	//subPasses[0].pInputAttachments = VK_NULL_HANDLE;
	subPasses[0].colorAttachmentCount = subPass0ColorAttachmentReferences.size();
	subPasses[0].pColorAttachments = subPass0ColorAttachmentReferences.data();
	//subPasses[0].pResolveAttachments;
	subPasses[0].pDepthStencilAttachment = &subPass0DepthStencilAttachmentReference;
	//subPasses[0].preserveAttachmentCount;
	//subPasses[0].pPreserveAttachments;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = subPasses.size();
	renderPassCreateInfo.pSubpasses = subPasses.data();
	//renderPassCreateInfo.dependencyCount;
	//renderPassCreateInfo.pDependencies;

	ErrorCheck(vkCreateRenderPass(m_Renderer->GetVulkanDevice(), &renderPassCreateInfo, nullptr, &m_RenderPass));
}

void Window::DestroyRenderPass()
{
	vkDestroyRenderPass(m_Renderer->GetVulkanDevice(), m_RenderPass, nullptr);
}

void Window::InitFramebuffers()
{
	m_Framebuffers.resize(m_SwapchainImageCount);
	for (uint32_t i = 0; i < m_SwapchainImageCount; ++i)
	{
		std::array<VkImageView, 2> attachments{};
		attachments[0] = m_DepthStencilImageView;
		attachments[1] = m_ImageViews[i];

		VkFramebufferCreateInfo framebufferCreateInfo {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		//framebufferCreateInfo.pNext;
		//framebufferCreateInfo.flags;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.attachmentCount = attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = m_Width;
		framebufferCreateInfo.height = m_Height;
		framebufferCreateInfo.layers = 1;

		ErrorCheck(vkCreateFramebuffer(m_Renderer->GetVulkanDevice(), &framebufferCreateInfo, nullptr, &m_Framebuffers[i]));
	}
}

void Window::DestroyFramebuffers()
{
	for (VkFramebuffer framebuffers : m_Framebuffers)
		vkDestroyFramebuffer(m_Renderer->GetVulkanDevice(), framebuffers, nullptr);
}

void Window::InitSyncronizations()
{
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//fenceCreateInfo.pNext;
	//fenceCreateInfo.flags;
	vkCreateFence(m_Renderer->GetVulkanDevice(), &fenceCreateInfo, nullptr, &m_SwapchainImageAvailable);
}

void Window::DestroySyncronizations()
{
	vkDestroyFence(m_Renderer->GetVulkanDevice(), m_SwapchainImageAvailable, nullptr);
}

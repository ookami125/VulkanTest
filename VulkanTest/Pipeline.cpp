
#include "BUILD_OPTIONS.h"
#include "Pipeline.h"




Pipeline::Pipeline(Renderer * renderer, Window * window, const std::string & name)
{
	m_Window = window;
	m_Renderer = renderer;
	m_GPU = renderer->GetVulkanPhysicalDevice();
	m_Device = renderer->GetVulkanDevice();
	m_Queue = renderer->GetVulkanQueue();
	m_Name = name;

	auto filepath = BUILD_PIPELINE_DIRECTORY + m_Name;
	{
		std::ifstream file(filepath + "/vert.spv", std::ifstream::binary | std::ifstream::ate);
		if (!file.is_open())
			abort();

		size_t file_size = file.tellg();
		std::vector<char> code(file_size);

		file.seekg(0);
		file.read(code.data(), file_size);
		file.close();

		VkShaderModuleCreateInfo shader_create_info_vertex{};
		shader_create_info_vertex.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info_vertex.codeSize = code.size();
		shader_create_info_vertex.pCode = reinterpret_cast<uint32_t*>(code.data());
		vkCreateShaderModule(m_Device, &shader_create_info_vertex, nullptr, &m_ShaderModuleVertex);
	}
	{
		std::ifstream file(filepath + "/frag.spv", std::ifstream::binary | std::ifstream::ate);
		if (!file.is_open())
			abort();

		auto file_size = file.tellg();
		std::vector<char> code(file_size);

		file.seekg(0);
		file.read(code.data(), file_size);
		file.close();

		VkShaderModuleCreateInfo shader_create_info_vertex{};
		shader_create_info_vertex.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info_vertex.codeSize = code.size();
		shader_create_info_vertex.pCode = reinterpret_cast<uint32_t*>(code.data());
		vkCreateShaderModule(m_Device, &shader_create_info_vertex, nullptr, &m_ShaderModuleFragment);
	}

	VkPipelineShaderStageCreateInfo shader_stage_create_infos[2]{ {},{} };
	shader_stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_create_infos[0].module = m_ShaderModuleVertex;
	shader_stage_create_infos[0].pName = "main";

	shader_stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_create_infos[1].module = m_ShaderModuleFragment;
	shader_stage_create_infos[1].pName = "main";

	VkVertexInputBindingDescription vertex_binding_description{};
	vertex_binding_description.binding = 0;
	vertex_binding_description.stride = sizeof(Mesh_Vertex);
	vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertex_attribute_description[]{ {},{} };
	vertex_attribute_description[0].binding = 0;
	vertex_attribute_description[0].location = 0;
	vertex_attribute_description[0].offset = 0;
	vertex_attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	/*
	vertex_attribute_description[ 1 ].binding			= 0;
	vertex_attribute_description[ 1 ].location			= 1;
	vertex_attribute_description[ 1 ].offset			= 12;
	vertex_attribute_description[ 1 ].format			= VK_FORMAT_R32G32B32_SFLOAT;
	*/
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
	vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
	vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_description;
	vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
	vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_description;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
	input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkViewport viewport{
		0.0f, 0.0f,
		float(m_Window->GetVulkanSurfaceSize().width), float(m_Window->GetVulkanSurfaceSize().height),
		0.0f, 1.0f
	};

	VkRect2D scissor{
		{ 0, 0 },
		{ m_Window->GetVulkanSurfaceSize().width, m_Window->GetVulkanSurfaceSize().height }
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterization_create_info{};
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.depthClampEnable = VK_FALSE;
	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_create_info.cullMode = VK_CULL_MODE_NONE;
	rasterization_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	rasterization_create_info.depthBiasConstantFactor = 0;
	rasterization_create_info.depthBiasClamp = 0;
	rasterization_create_info.depthBiasSlopeFactor = 0;
	rasterization_create_info.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
	multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state_create_info.sampleShadingEnable = VK_FALSE;
	multisample_state_create_info.minSampleShading = 1.0f;
	multisample_state_create_info.pSampleMask = nullptr;
	multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_state_create_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
	depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
	depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
	depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
	depth_stencil_state_create_info.minDepthBounds = 0;
	depth_stencil_state_create_info.maxDepthBounds = 0;
	depth_stencil_state_create_info.front.failOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	depth_stencil_state_create_info.front.depthFailOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state_create_info.front.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
	depth_stencil_state_create_info.front.compareMask = UINT32_MAX;
	depth_stencil_state_create_info.front.writeMask = UINT32_MAX;
	depth_stencil_state_create_info.front.reference = 1;
	depth_stencil_state_create_info.back = depth_stencil_state_create_info.front;

	VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
	color_blend_attachment_state.blendEnable = VK_FALSE;
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_SUBTRACT;
	color_blend_attachment_state.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
	color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable = VK_FALSE;
	color_blend_state_create_info.logicOp = VK_LOGIC_OP_NO_OP;
	color_blend_state_create_info.attachmentCount = 1;
	color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
	color_blend_state_create_info.blendConstants[0] = 1.0f;
	color_blend_state_create_info.blendConstants[1] = 1.0f;
	color_blend_state_create_info.blendConstants[2] = 1.0f;
	color_blend_state_create_info.blendConstants[3] = 1.0f;

	std::vector<VkDynamicState> dynamic_states{};
	VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
	dynamic_state_create_info.pDynamicStates = dynamic_states.data();

	std::vector<VkPushConstantRange> range(1);
	range[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	range[0].offset = 0;
	range[0].size = sizeof(float);

	range.clear();

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pSetLayouts = nullptr;
	pipeline_layout_create_info.pushConstantRangeCount = range.size();
	pipeline_layout_create_info.pPushConstantRanges = range.data();
	vkCreatePipelineLayout(m_Device, &pipeline_layout_create_info, nullptr, &m_PipelineLayout);

	VkGraphicsPipelineCreateInfo pipeline_create_info{};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = shader_stage_create_infos;
	pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
	pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
	pipeline_create_info.pTessellationState = nullptr;
	pipeline_create_info.pViewportState = &viewport_state_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_create_info;
	pipeline_create_info.pMultisampleState = &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
	pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
	if (dynamic_states.size()) {
		pipeline_create_info.pDynamicState = &dynamic_state_create_info;
	}
	pipeline_create_info.layout = m_PipelineLayout;
	pipeline_create_info.renderPass = m_Window->GetVulkanRenderPass();
	pipeline_create_info.subpass = 0;
	pipeline_create_info.basePipelineIndex = -1;
	vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_Pipeline);
}


Pipeline::~Pipeline()
{
	vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
	vkDestroyShaderModule(m_Device, m_ShaderModuleVertex, nullptr);
	vkDestroyShaderModule(m_Device, m_ShaderModuleFragment, nullptr);
}

VkPipeline Pipeline::GetVulkanPipeline()
{
	return m_Pipeline;
}

const std::string & Pipeline::GetName()
{
	return m_Name;
}
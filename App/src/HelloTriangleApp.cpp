#include "HelloTriangleApp.h"

#include <filesystem>
#include <string>

#include "core/Helpers.h"

void HelloTriangleApplication::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
}

void HelloTriangleApplication::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FrameBufferResizeCallback);
}

void HelloTriangleApplication::InitVulkan()
{
    CreateInstance();
    SetUpDebugMessenger();
    CreateSurface();
    PickPhysicalDevide();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void HelloTriangleApplication::MainLoop()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        DrawFrame();
    }

    m_device.waitIdle();
}

void HelloTriangleApplication::CleanUp()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void HelloTriangleApplication::CreateInstance()
{
    constexpr vk::ApplicationInfo appInfo
    {
        .pApplicationName   = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0),
        .pEngineName        = "SEngine",
        .engineVersion      = VK_MAKE_VERSION( 1, 0, 0),
        .apiVersion         = vk::ApiVersion14
    };

    std::vector<char const *> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

    // Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties    = m_context.enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
	    [&layerProperties](auto const &requiredLayer) {
		    return std::ranges::none_of(layerProperties,
		        [requiredLayer](auto const &layerProperty) 
                    { return strcmp(layerProperty.layerName, requiredLayer) == 0; }
                );
	    });
	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}

    auto requiredExtensions = GetRequiredInstanceExtensions();

    // Check if the required GLFW extensions are supported by the Vulkan implementation.
    auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt =
	    std::ranges::find_if(requiredExtensions,
	        [&extensionProperties](auto const &requiredExtension) 
            {
		        return std::ranges::none_of(extensionProperties,
		            [requiredExtension](auto const &extensionProperty) 
                    { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
	        });
	if (unsupportedPropertyIt != requiredExtensions.end())
	{
		throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
	}

    vk::InstanceCreateInfo createInfo
    {
        .pApplicationInfo           = &appInfo,
        .enabledLayerCount          = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames        = requiredLayers.data(),
		.enabledExtensionCount      = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames    = requiredExtensions.data()   
    };
    m_instance = vk::raii::Instance(m_context, createInfo);
}

std::vector<const char *> HelloTriangleApplication::GetRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if(enableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

void HelloTriangleApplication::SetUpDebugMessenger()
{
    if (!enableValidationLayers) return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
                                                                          .messageType     = messageTypeFlags,
                                                                          .pfnUserCallback = &HelloTriangleApplication::debugCallback};
    m_debug_messenger = m_instance.createDebugUtilsMessengerEXT( debugUtilsMessengerCreateInfoEXT );
}

void HelloTriangleApplication::CreateSurface()
{
    VkSurfaceKHR surface;
    if(glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surface) != 0) {
        throw std::runtime_error("Failed to create window surface!");
    }
    m_surface = vk::raii::SurfaceKHR(m_instance, surface);
}

void HelloTriangleApplication::PickPhysicalDevide()
{
    auto physicalDevices = m_instance.enumeratePhysicalDevices();
    if(physicalDevices.empty()) throw std::runtime_error("Failed to find GPUs with Vulkan Support :(");

    auto const devIter = std::ranges::find_if( physicalDevices, [&]( auto const & physicalDevice ) { return IsDeviceSuitable( physicalDevice ); } );
    if ( devIter == physicalDevices.end() )
    {
        throw std::runtime_error( "Failed to find a suitable GPU!" );
    }
    m_physical_device = *devIter;
}

bool HelloTriangleApplication::IsDeviceSuitable(const vk::raii::PhysicalDevice &PhysicalDevice)
{
    bool supportsVulkan1_3 = PhysicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
    auto queueFamilies = PhysicalDevice.getQueueFamilyProperties();
    bool supportsGraphics =
        std::ranges::any_of(queueFamilies, [](auto const &qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });  

    auto availableDeviceExtensions = PhysicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions =
    std::ranges::all_of( m_required_device_extension,
    [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
    {
        return std::ranges::any_of( availableDeviceExtensions,
            [requiredDeviceExtension]( auto const & availableDeviceExtension )
            { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
    });

    auto features                 = PhysicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures = 
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

void HelloTriangleApplication::CreateLogicalDevice()
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physical_device.getQueueFamilyProperties();

	// get the first index into queueFamilyProperties which supports both graphics and present

	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
		    m_physical_device.getSurfaceSupportKHR(qfpIndex, *m_surface))
		{
			// found a queue family that supports both graphics and present
			m_queue_index = qfpIndex;
			break;
		}
	}
	if (m_queue_index == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}    

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
        {},                               // vk::PhysicalDeviceFeatures2 (empty for now)
        {.shaderDrawParameters = true },  // Required for SPIR-V DrawParameters capability
        {.synchronization2 = true, .dynamicRendering = true}, // Enable dynamic rendering from Vulkan 1.3
        {.extendedDynamicState = true }   // Enable extended dynamic state from the extension
    };

    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
        .queueFamilyIndex = m_queue_index,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(m_required_device_extension.size()),
        .ppEnabledExtensionNames = m_required_device_extension.data()
    };
    m_device = vk::raii::Device( m_physical_device, deviceCreateInfo );
    m_graphics_queue = vk::raii::Queue( m_device, m_queue_index, 0 );
}

void HelloTriangleApplication::CreateSwapChain()
{
    vk::SurfaceCapabilitiesKHR surfaceCapabilities  = m_physical_device.getSurfaceCapabilitiesKHR(*m_surface);
	m_swap_chain_extent                             = ChooseSwapExtent(surfaceCapabilities);
	uint32_t minImageCount                          = ChooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = m_physical_device.getSurfaceFormatsKHR(*m_surface);
	m_swap_chain_surface_format                        = ChooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = m_physical_device.getSurfacePresentModesKHR(*m_surface);
	vk::PresentModeKHR              presentMode           = ChooseSwapPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface          = *m_surface,
	                                               .minImageCount    = minImageCount,
	                                               .imageFormat      = m_swap_chain_surface_format.format,
	                                               .imageColorSpace  = m_swap_chain_surface_format.colorSpace,
	                                               .imageExtent      = m_swap_chain_extent,
	                                               .imageArrayLayers = 1,
	                                               .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
	                                               .imageSharingMode = vk::SharingMode::eExclusive,
	                                               .preTransform     = surfaceCapabilities.currentTransform,
	                                               .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
	                                               .presentMode      = presentMode,
	                                               .clipped          = true};

	m_swap_chain       = vk::raii::SwapchainKHR(m_device, swapChainCreateInfo);
	m_swap_chain_images = m_swap_chain.getImages();
}

void HelloTriangleApplication::RecreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    m_device.waitIdle();
    CleanUpSwapChain();
    CreateSwapChain();
    CreateImageViews();
}

void HelloTriangleApplication::CleanUpSwapChain()
{
    m_swap_chain_image_views.clear();
    m_swap_chain = nullptr;
}

vk::Extent2D HelloTriangleApplication::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &Capabilities)
{
    if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return Capabilities.currentExtent;
	}
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	return {
	    std::clamp<uint32_t>(width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width),
	    std::clamp<uint32_t>(height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height)};
}

uint32_t HelloTriangleApplication::ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &SurfaceCapabilities)
{
    auto minImageCount = std::max(3u, SurfaceCapabilities.minImageCount);
	if ((0 < SurfaceCapabilities.maxImageCount) && (SurfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = SurfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}

vk::SurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &AvailableFormats)
{
    assert(!AvailableFormats.empty());
	const auto formatIt = std::ranges::find_if(
	    AvailableFormats,
	    [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
	return formatIt != AvailableFormats.end() ? *formatIt : AvailableFormats[0];
}

vk::PresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &AvailablePresentModes)
{
    assert(std::ranges::any_of(AvailablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(AvailablePresentModes,
	                           [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
	           vk::PresentModeKHR::eMailbox :
	           vk::PresentModeKHR::eFifo;
}

void HelloTriangleApplication::CreateImageViews()
{
    assert(m_swap_chain_image_views.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = m_swap_chain_surface_format.format,
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    };

    for(auto& image : m_swap_chain_images)
    {
        imageViewCreateInfo.image = image;
        m_swap_chain_image_views.emplace_back(m_device, imageViewCreateInfo);
    }
}

void HelloTriangleApplication::CreateGraphicsPipeline()
{
    auto resPath = SE::GetResourcesDirectory() / "shaders" / "triangle_shader.slang.spv";
    std::vector<char> shaderCode = SE::ReadFile(resPath.string());
    vk::raii::ShaderModule shaderModule = CreateShaderModule(shaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo { 
        .stage = vk::ShaderStageFlagBits::eVertex, 
        .module = shaderModule, 
        .pName = "vertMain"
    };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo { 
        .stage = vk::ShaderStageFlagBits::eFragment, 
        .module = shaderModule, 
        .pName = "fragMain"
    };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributesDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo
    {   .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescriptions.size() ),
        .pVertexAttributeDescriptions    = attributeDescriptions.data() 
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{  .topology = vk::PrimitiveTopology::eTriangleList };
    vk::PipelineViewportStateCreateInfo viewportState{  .viewportCount = 1, .scissorCount = 1  };
    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False, .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill, .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise, .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f, .lineWidth = 1.0f
    };
    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable    = vk::False,
        .colorWriteMask =   vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = vk::False, 
        .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment
    };

    std::vector dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{ 
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data() 
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{  .setLayoutCount = 0, .pushConstantRangeCount = 0 };

    m_pipeline_layout = vk::raii::PipelineLayout( m_device, pipelineLayoutInfo );

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
	    {   .stageCount          = 2,
	        .pStages             = shaderStages,
	        .pVertexInputState   = &vertexInputInfo,
	        .pInputAssemblyState = &inputAssembly,
	        .pViewportState      = &viewportState,
	        .pRasterizationState = &rasterizer,
	        .pMultisampleState   = &multisampling,
	        .pColorBlendState    = &colorBlending,
	        .pDynamicState       = &dynamicState,
	        .layout              = m_pipeline_layout,
	        .renderPass          = nullptr},
	    {   .colorAttachmentCount = 1, .pColorAttachmentFormats = &m_swap_chain_surface_format.format}
    };
    
    m_graphics_pipeline = vk::raii::Pipeline(m_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

vk::raii::ShaderModule HelloTriangleApplication::CreateShaderModule(const std::vector<char> &Code) const
{
    vk::ShaderModuleCreateInfo createInfo{ .codeSize = Code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(Code.data()) };
    vk::raii::ShaderModule shaderModule{ m_device, createInfo };
    return shaderModule;
}

void HelloTriangleApplication::CreateCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{ .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = m_queue_index };
    m_command_pool = vk::raii::CommandPool(m_device, poolInfo);
}

void HelloTriangleApplication::CreateBuffer(vk::DeviceSize Size, 
    vk::BufferUsageFlags Usage, vk::MemoryPropertyFlags Properties,
    vk::raii::Buffer& Buffer, vk::raii::DeviceMemory& BufferMemory)
{
    vk::BufferCreateInfo bufferInfo{ 
        .size = Size,
        .usage = Usage, 
        .sharingMode = vk::SharingMode::eExclusive 
    };
    Buffer = vk::raii::Buffer(m_device, bufferInfo);
    vk::MemoryRequirements memRequirements = Buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{ 
        .allocationSize = memRequirements.size, 
        .memoryTypeIndex = 
        FindMemoryType(memRequirements.memoryTypeBits, Properties) 
    };
    BufferMemory = vk::raii::DeviceMemory(m_device, allocInfo);
    Buffer.bindMemory(*BufferMemory, 0);
}

void HelloTriangleApplication::CreateVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
    vk::raii::Buffer stagingBuffer = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory = nullptr;
    CreateBuffer(bufferSize, 
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory
    );

    void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, m_vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    CreateBuffer(bufferSize, 
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        m_vertex_buffer, m_vertex_buffer_memory
    );

    CopyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);
}

void HelloTriangleApplication::CopyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocInfo{.commandPool = m_command_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
	vk::raii::CommandBuffer       commandCopyBuffer = std::move(m_device.allocateCommandBuffers(allocInfo).front());
	commandCopyBuffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
	commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
	commandCopyBuffer.end();
	m_graphics_queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
	m_graphics_queue.waitIdle();
}

void HelloTriangleApplication::CreateIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, m_indices.data(), (size_t) bufferSize);
    stagingBufferMemory.unmapMemory();

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, 
        vk::MemoryPropertyFlagBits::eDeviceLocal, m_index_buffer, m_index_buffer_memory);

    CopyBuffer(stagingBuffer, m_index_buffer, bufferSize);
}

uint32_t HelloTriangleApplication::FindMemoryType(uint32_t TypeFilter, vk::MemoryPropertyFlags Properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = m_physical_device.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((TypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
    return 0;
}

void HelloTriangleApplication::CreateCommandBuffers()
{
    m_command_buffers.clear();
    vk::CommandBufferAllocateInfo allocInfo{.commandPool = m_command_pool, 
        .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
	m_command_buffers = vk::raii::CommandBuffers(m_device, allocInfo);
}

void HelloTriangleApplication::RecordCommandBuffer(uint32_t ImageIndex)
{
    auto& commandBuffer = m_command_buffers[m_frame_index];
    commandBuffer.begin( {} );

    TransitionImageLayout(
        ImageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = m_swap_chain_image_views[ImageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };
    vk::RenderingInfo renderingInfo = {
        .renderArea = { .offset = { 0, 0 }, .extent = m_swap_chain_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphics_pipeline);
    
    commandBuffer.setViewport(
        0, 
        vk::Viewport(
            0.0f, 0.0f, 
            static_cast<float>(m_swap_chain_extent.width), static_cast<float>(m_swap_chain_extent.height),
            0.0, 1.0f
        )
    );
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_swap_chain_extent));

    commandBuffer.bindVertexBuffers(0, *m_vertex_buffer, {0});
    commandBuffer.bindIndexBuffer(*m_index_buffer, 0, vk::IndexType::eUint16);
    
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);

    commandBuffer.endRendering();

    TransitionImageLayout(
        ImageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
        {},                                                     // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
    );
 
    commandBuffer.end();
}

void HelloTriangleApplication::TransitionImageLayout(uint32_t imageIndex, vk::ImageLayout old_layout,
    vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask, vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask)
{
    vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask        = src_stage_mask,
		.srcAccessMask       = src_access_mask,
		.dstStageMask        = dst_stage_mask,
		.dstAccessMask       = dst_access_mask,
		.oldLayout           = old_layout,
		.newLayout           = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image               = m_swap_chain_images[imageIndex],
		.subresourceRange    = 
        {
		   .aspectMask     = vk::ImageAspectFlagBits::eColor,
		   .baseMipLevel   = 0,
		   .levelCount     = 1,
		   .baseArrayLayer = 0,
		   .layerCount     = 1
        }
    };

	vk::DependencyInfo dependency_info = {
	    .dependencyFlags         = {},
	    .imageMemoryBarrierCount = 1,
	    .pImageMemoryBarriers    = &barrier};

	m_command_buffers[m_frame_index].pipelineBarrier2(dependency_info);
}

void HelloTriangleApplication::CreateSyncObjects()
{
    assert(m_present_complete_sems.empty() && m_render_finished_sems.empty() && m_in_flight_fences.empty());

    for(size_t i = 0; i < m_swap_chain_images.size(); i++)
    {
        m_render_finished_sems.emplace_back(m_device, vk::SemaphoreCreateInfo());
    }

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_present_complete_sems.emplace_back(m_device, vk::SemaphoreCreateInfo());
        m_in_flight_fences.emplace_back(m_device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void HelloTriangleApplication::DrawFrame()
{
    auto fenceResult = m_device.waitForFences(*m_in_flight_fences[m_frame_index], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fence!");
	}

    auto [result, imageIndex] = m_swap_chain.acquireNextImage(UINT64_MAX, *m_present_complete_sems[m_frame_index], nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_device.resetFences(*m_in_flight_fences[m_frame_index]);

    m_command_buffers[m_frame_index].reset();
    RecordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1, .pWaitSemaphores = &*m_present_complete_sems[m_frame_index],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1, .pCommandBuffers = &*m_command_buffers[m_frame_index],
        .signalSemaphoreCount = 1, .pSignalSemaphores = &*m_render_finished_sems[imageIndex]
    };
    m_graphics_queue.submit(submitInfo, *m_in_flight_fences[m_frame_index]);

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1, .pWaitSemaphores = &*m_render_finished_sems[imageIndex],
        .swapchainCount = 1, .pSwapchains = &*m_swap_chain,
        .pImageIndices = &imageIndex
    };
    result = m_graphics_queue.presentKHR(presentInfoKHR);
    if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_frame_buffer_resized)
	{
		m_frame_buffer_resized = false;
		RecreateSwapChain();
	}
	else
	{
		// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
		assert(result == vk::Result::eSuccess);
	}

    m_frame_index = (m_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL HelloTriangleApplication::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT Severity,
     vk::DebugUtilsMessageTypeFlagsEXT Type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    if (Severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || Severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cerr << "validation layer: type " << to_string(Type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

void HelloTriangleApplication::FrameBufferResizeCallback(GLFWwindow *Window, int Width, int Height)
{
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(Window));
    app->WindowResize();
}

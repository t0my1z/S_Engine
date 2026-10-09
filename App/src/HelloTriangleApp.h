#pragma once

#include <iostream>
#include <stdexcept> //Catch exceptions
#include <cstdlib> //EXIT_SUCCESS && EXIT_FAILURE

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stb_image.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
    glm::vec2 Pos;
    glm::vec3 Color;

    static vk::VertexInputBindingDescription GetBindingDescription()
    {
        return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
    }

    static std::array<vk::VertexInputAttributeDescription, 2> GetAttributesDescriptions()
    {
        return {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, Pos)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Color))
        };
    }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class HelloTriangleApplication 
{
public:
    void Run();
    void WindowResize() { m_frame_buffer_resized = true; }
private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void CleanUp();

    void CreateInstance();
    std::vector<const char*> GetRequiredInstanceExtensions();
    void SetUpDebugMessenger();
    void CreateSurface();

    void PickPhysicalDevide();
    //Can implement this however you want, for example, giving a score to each device and sorting by the one 
    //with the highest score using some arbitrary params to set the score for each one (Discrete GPU, max size of textures, etc)
    bool IsDeviceSuitable(const vk::raii::PhysicalDevice& PhysicalDevice);

    void CreateLogicalDevice();

    void CreateSwapChain();
    void RecreateSwapChain();
    void CleanUpSwapChain();
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &Capabilities);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &SurfaceCapabilities);
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &AvailableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &AvailablePresentModes);

    void CreateImageViews();
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> CreateImage(uint32_t width, uint32_t height,
        vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
    void TransitionImageLayout(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void CopyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);
    vk::raii::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer);

    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& Code) const;

    void CreateCommandPool();
    void CreateTextureImage();

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(vk::DeviceSize Size, vk::BufferUsageFlags Usage,
        vk::MemoryPropertyFlags Properties);
    void CreateVertexBuffer();
    void CopyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    uint32_t FindMemoryType(uint32_t TypeFilter, vk::MemoryPropertyFlags Properties);
    void CreateCommandBuffers();

    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void RecordCommandBuffer(uint32_t ImageIndex);
    void TransitionImageLayout(uint32_t imageIndex,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask);

    void CreateSyncObjects();

    void DrawFrame();

    void UpdateUniformBuffer(uint32_t Frame);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT Severity,
        vk::DebugUtilsMessageTypeFlagsEXT Type,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *
    );

    static void FrameBufferResizeCallback(GLFWwindow* Window, int Width, int Height);

private:
    GLFWwindow*                             m_window = nullptr;

    vk::raii::Context                       m_context;
    vk::raii::Instance                      m_instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT        m_debug_messenger = nullptr;
    vk::raii::SurfaceKHR                    m_surface = nullptr;

    vk::raii::PhysicalDevice                m_physical_device = nullptr;
    std::vector<const char*>                m_required_device_extension = {vk::KHRSwapchainExtensionName};
    vk::raii::Device                        m_device = nullptr;
    uint32_t                                m_queue_index = ~0;
    vk::raii::Queue                         m_graphics_queue = nullptr;

    vk::raii::SwapchainKHR                  m_swap_chain      = nullptr;
	std::vector<vk::Image>                  m_swap_chain_images;
	vk::SurfaceFormatKHR                    m_swap_chain_surface_format;
	vk::Extent2D                            m_swap_chain_extent;
	std::vector<vk::raii::ImageView>        m_swap_chain_image_views;
    
    vk::raii::DescriptorSetLayout           m_descriptor_set_layout = nullptr;
    vk::raii::PipelineLayout                m_pipeline_layout = nullptr;
    vk::raii::Pipeline                      m_graphics_pipeline = nullptr;

    vk::raii::CommandPool                   m_command_pool = nullptr;
    std::vector<vk::raii::CommandBuffer>    m_command_buffers; 

    std::vector<vk::raii::Semaphore>        m_present_complete_sems;
    std::vector<vk::raii::Semaphore>        m_render_finished_sems;
    std::vector<vk::raii::Fence>            m_in_flight_fences;
    uint32_t                                m_frame_index = 0;
    bool                                    m_frame_buffer_resized = false;

    vk::raii::Buffer                        m_vertex_buffer = nullptr;
    vk::raii::DeviceMemory                  m_vertex_buffer_memory = nullptr;
    vk::raii::Buffer                        m_index_buffer = nullptr;
    vk::raii::DeviceMemory                  m_index_buffer_memory = nullptr;

    std::vector<vk::raii::Buffer>           m_uniform_buffers;
    std::vector<vk::raii::DeviceMemory>     m_uniform_buffers_memory;
    std::vector<void *>                     m_uniform_buffers_mapped;

    vk::raii::DescriptorPool                m_descriptor_pool = nullptr;
    std::vector<vk::raii::DescriptorSet>    m_descriptor_sets;

    vk::raii::Image                         m_texture_image         = nullptr;
    vk::raii::DeviceMemory                  m_texture_image_memory  = nullptr;

    const std::vector<Vertex>               m_vertices = {
                                                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                                {{0.5f , -0.5f}, {0.0f, 1.0f, 0.0f}},
                                                {{0.5f ,  0.5f}, {0.0f, 0.0f, 1.0f}},
                                                {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
                                            };
    
    const std::vector<uint16_t>             m_indices = {
                                                0, 1, 2, 2, 3, 0
                                            };
};              
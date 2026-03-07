#include <iostream>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>

void TestVulkan() {
    uint32_t version = 0;
    VkResult res = vkEnumerateInstanceVersion(&version);
    if (res == VK_SUCCESS) {
        std::cout << "Vulkan API Version: "
                  << VK_VERSION_MAJOR(version) << "."
                  << VK_VERSION_MINOR(version) << "."
                  << VK_VERSION_PATCH(version) << "\n";
    } else {
        std::cout << "Failed to query Vulkan version.\n";
    }
}

int main()
{
    std::cout << "Test 2" << std::endl;

    TestVulkan();

    glm::vec3 testVec{1.0, 1.0, 1.0f};

    std::cout << "X:" << testVec.x << ", Y:" << testVec.y << ", Z:" << testVec.z << std::endl;

    return 1;
}
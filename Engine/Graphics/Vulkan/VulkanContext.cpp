#include <Engine/Graphics/Vulkan/VulkanContext.hpp>

#include <Engine/Graphics/Vulkan/Device.hpp>
#include <Engine/Graphics/Window.hpp>

namespace giraphics {

    VulkanContext::VulkanContext(Window& window)
        : m_Window(window)
    {
        m_Device = std::make_unique<Device>(m_Window);
    }

    VulkanContext::~VulkanContext() {}

}  // namespace giraphics

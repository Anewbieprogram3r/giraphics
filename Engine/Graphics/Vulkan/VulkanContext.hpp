#pragma once

#include <memory>

namespace giraphics {
    class Device;
    class Window;
    class VulkanContext {
    public:
        VulkanContext(Window& m_Window);
        ~VulkanContext();

    private:
        Window& m_Window;
        std::unique_ptr<Device> m_Device{};
    };
}  // namespace giraphics

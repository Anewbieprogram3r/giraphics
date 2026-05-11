#pragma once

#include <vulkan/vulkan.hpp>

#include <Engine/Graphics/GiraphicsPCH.hpp>

namespace giraphics {
    // Forward declaration of Window class
    class Window;
    class Device {
    public:
        // Constructor that takes a reference to the Window class, used for surface creation in Vulkan.
        Device(Window& window);

        // Destructor that handles cleanup of Vulkan resources, including the logical device and instance.
        ~Device();

    private:
        // Instance specific function
        bool isRequestedValidationLayersAvailable();
        void createVulkanInstance();
        void enumerateInstanceExtensions();
        std::vector<const char*> getInstanceExtensions();

        // Vulkan debugging capabilities specific function
        VkResult enableDebuggingCapabilities();

        // Vulkan instance handle, representing the connection between the application and the Vulkan library.
        VkInstance m_Instance;

        // Flag indicating whether validation layers are enabled for debugging purposes.
        const bool m_isValidationLayerON = true;

        // Flag indicating whether validation layers are actually available on the system.
        bool m_areValidationLayersAvailable = false;

        // List of requested validation layers (e.g., VK_LAYER_KHRONOS_validation) for debugging purposes.
        const std::vector<const char*> m_RequestedValidationLayers = { "VK_LAYER_KHRONOS_validation" };

        // Vulkan debug messenger for handling validation layer messages and debug output.
        VkDebugUtilsMessengerEXT m_DebugMessenger;
        PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr;

        // Reference to the window object, used for creating the Vulkan surface.
        Window& m_Window;
    };

}  // namespace giraphics

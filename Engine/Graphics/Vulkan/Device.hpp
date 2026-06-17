#pragma once

#include <vulkan/vulkan.hpp>

#include <Engine/Graphics/GiraphicsPCH.hpp>

namespace giraphics {
    class Window;
    // Forward declaration of Window class
    class Window;

    // The Device class manages the creation and control of the Vulkan device.
    // It encapsulates Vulkan instance, physical and logical device selection, and provides access to device queues.
    class Device : public NonCopyable, public NonMovable {
    public:
        // Constructor that takes a reference to the Window class, used for surface creation in Vulkan.
        Device(Window& window);

        // Destructor that handles cleanup of Vulkan resources, including the logical device and instance.
        ~Device();

        // Getter functions to provide access to Vulkan components:
        VkInstance instance() const { return m_Instance; }            // Returns the Vulkan instance handle.
        VkPhysicalDevice physicalDevice() const { return m_PhysicalDevice; }  // Returns the Vulkan physical device handle.
        VkDevice device() const { return m_LogicalDevice; }            // Returns the Vulkan logical device handle.
        VkQueue queue() const { return m_GraphicsQueue; }             // Returns the Vulkan graphics queue.
        VkCommandPool commandPool() const { return m_CommandPool; }   // Returns the Vulkan command pool.
        VkSurfaceKHR surface() const { return m_SurfaceKHR; }         // Returns the Vulkan surface for rendering.

        // Functions to manage command buffer lifecycle:
        VkCommandBuffer beginCommandBuffer();   // Begins recording a command buffer for Vulkan operations.
        void endCommandBuffer(VkCommandBuffer commandBuffer);  // Ends recording of the command buffer.

        // Properties of the physical device, useful for querying device-specific details.
        VkPhysicalDeviceProperties properties;

        // Initializes the Vulkan device, creating the instance, logical device, and setting up queues.
        void initialize();

        // Helper functions for Vulkan instance creation and setup:
        bool isRequestedValidationLayersAvailable();  // Checks if the requested validation layers are available on the system.
        void createVulkanInstance();                  // Creates a Vulkan instance for the application.
        void enumerateInstanceExtensions();           // Enumerates and retrieves the supported instance extensions.
        std::vector<const char*> getInstanceExtensions();  // Returns the required instance extensions for Vulkan.

        // Vulkan debugging support function:
        VkResult enableDebuggingCapabilities();       // Enables Vulkan debug utilities (e.g., validation layers and debug messages).

        // Functions specific to window surface creation:
        void createWindowSurface();                   // Creates a Vulkan surface linked to the window.

        // Functions for physical device selection and logical device creation:
        void enumerateAndSelectPhysicalDevice();      // Enumerates and selects a physical device with the necessary features.
        void createLogicalDeviceAndConfigureQueues();                   // Creates a logical device and retrieves the required queues from the physical device.
        void createCommandPool();                     // Creates a command pool for managing command buffers.

        // Helper function to check if the physical device supports the required extensions.
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

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
        PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT = nullptr;  // Function pointer for creating the debug messenger.
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr; // Function pointer for destroying the debug messenger.

        // Reference to the window object, used for creating the Vulkan surface.
        Window& m_Window;

        // Vulkan command pool used to allocate and manage command buffers.
        VkCommandPool m_CommandPool;

        // Vulkan physical device handle, representing the selected GPU.
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

        // Vulkan logical device handle, representing the interface with the GPU.
        VkDevice m_LogicalDevice = VK_NULL_HANDLE;

        // List of required device extensions (e.g., VK_KHR_SWAPCHAIN_EXTENSION_NAME) for swapchain support.
        const std::vector<const char*> m_RequestedDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        // Vulkan surface handle, used for rendering to the window.
        VkSurfaceKHR m_SurfaceKHR;

        // Vulkan queue for submitting graphics commands.
        VkQueue m_GraphicsQueue;

        // Friend classes that require access to private members of the Device class.
        friend class SwapChain;
        friend class Buffer;
    };

    // DeviceHelper struct provides utility functions to help with various Vulkan operations
    // related to queue families, memory types, and supported formats.
    struct DeviceHelper {
        // QueueFamilyIndices stores the indices for different queue families (graphics, compute, and transfer)
        // used by the Vulkan physical device.
        struct QueueFamilyIndices {
            uint32_t graphicsFamily = UINT32_MAX;  // Index for the graphics queue family
            uint32_t computeFamily = UINT32_MAX;   // Index for the compute queue family
            uint32_t transferFamily = UINT32_MAX;  // Index for the transfer queue family
        };

        // Queries queue families on a given physical device to find one that supports specific queue flags
        // (e.g., graphics, compute, transfer). Optionally, checks for surface compatibility (e.g., for presentation support).
        // Returns the index of the queue family that matches the flags.
        static uint32_t queryQueueFamilies(VkPhysicalDevice device, VkQueueFlags queryQueueFlags, VkSurfaceKHR surfaceKHR = VK_NULL_HANDLE);

        // Finds the appropriate memory type on a physical device that matches the required type filter and memory property flags.
        static uint32_t getMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };

}  // namespace giraphics
#include <Engine/Graphics/Vulkan/Device.hpp>
#include <Engine/Graphics/Window.hpp>
#include <Engine/Graphics/Vulkan/SwapChain.hpp>

namespace giraphics {
    /**
     * @brief Constructs the Device object and initializes Vulkan instance, device, and other necessary components.
     *
     * @param window Reference to the Window object used to create the Vulkan surface.
     */
    Device::Device(Window& window) : m_Window{ window } {
    }

    /**
     * @brief Destroys the Vulkan device and cleans up all associated resources.
     *
     * This includes destroying the command pool, logical device, debug messenger (if validation layers are enabled),
     * Vulkan surface, and Vulkan instance.
     */
    Device::~Device() {
        // Destroy the command pool
        vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);

        // Destroy the logical device
        vkDestroyDevice(m_LogicalDevice, nullptr);

        // Destroy the Vulkan debug messenger if validation layers are enabled
        if (m_isValidationLayerON && m_vkDestroyDebugUtilsMessengerEXT) {
            m_vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        // Destroy the Vulkan surface
        vkDestroySurfaceKHR(m_Instance, m_SurfaceKHR, nullptr);

        // Destroy the Vulkan instance
        vkDestroyInstance(m_Instance, nullptr);
    }

    /**
     * @brief Initializes the Vulkan device by setting up the Vulkan instance, selecting a physical device,
     * and creating a logical device with necessary features and command pools.
     */
    void Device::initialize() {
        m_areValidationLayersAvailable = isRequestedValidationLayersAvailable();   // Check if the requested validation layers are available
        createVulkanInstance();                   // Create the Vulkan instance
        enumerateInstanceExtensions();            // Enumerate Vulkan instance extensions
        enableDebuggingCapabilities();            // Enable debugging features if validation layers are enabled
        createWindowSurface();                    // Create the Vulkan window surface for rendering
        enumerateAndSelectPhysicalDevice();       // Find and select a suitable physical device (GPU)
        createLogicalDeviceAndConfigureQueues();  // Create the logical device from the selected physical device
        createCommandPool();                      // Create a command pool for managing command buffers
    }

    void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = DeviceHelper::getMemoryType(m_PhysicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
    }

    void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginCommandBuffer();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endCommandBuffer(commandBuffer);
    }

    /**
     * @brief Checks if the requested validation layers are available on the system.
     *
     * @return True if all requested validation layers are available; otherwise, false.
     *
     * @throws std::runtime_error If validation layers are enabled but not all requested layers are available.
     */
    bool Device::isRequestedValidationLayersAvailable() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);  // Get the number of available validation layers

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // Get all available validation layers

        // Loop through each requested validation layer and check if it exists
        for (const char* layerName : m_RequestedValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {  // Check if the requested layer is available
                    layerFound = true;
                    break;
                }
            }

            // If a requested validation layer is not found, log a warning but don't throw if validation layers are optional
            if (!layerFound) {
                if (m_isValidationLayerON) {
                    std::cerr << "Warning: Validation layer '" << layerName << "' is not available. Continuing without validation layers." << std::endl;
                }

                return false;
            }
        }

        return true;
    }

    /**
     * @brief Creates the Vulkan instance, which is the connection between the application and the Vulkan library.
     *
     * @throws std::runtime_error If the Vulkan instance creation fails.
     */
    void Device::createVulkanInstance() {
        // Set up application info to describe the application and the engine
        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Giraphics";       // Application name
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Application version
        applicationInfo.pEngineName = "Giraphics";               // Engine name
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Engine version
        applicationInfo.apiVersion = VK_API_VERSION_1_0;         // Specify the Vulkan API version

        // Set up the instance creation info
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef __APPLE__
        // If on Apple platforms, add portability flag to the instance creation
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        createInfo.pApplicationInfo = &applicationInfo;  // Attach the application info
        // Only enable validation layers if they are requested AND available
        if (m_isValidationLayerON && m_areValidationLayersAvailable) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_RequestedValidationLayers.size()); // Number of validation layers to enable
            createInfo.ppEnabledLayerNames = m_RequestedValidationLayers.data();  // Pointer to validation layer names
        } else {
            createInfo.enabledLayerCount = 0;  // No validation layers
            createInfo.ppEnabledLayerNames = nullptr;
        }

        // Get required extensions for the Vulkan instance
        auto extensions = getInstanceExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());  // Number of extensions to enable
        createInfo.ppEnabledExtensionNames = extensions.data();                       // Pointer to extension names

        // Create the Vulkan instance and check for errors
        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    /**
     * @brief Enumerates available instance extensions and checks if all required extensions are present.
     *
     * @throws std::runtime_error If required instance extensions are missing or enumeration fails.
     */
    void Device::enumerateInstanceExtensions() {
        uint32_t extensionCount = 0;

        // Get the number of available extensions
        VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate instance extension properties");
        }

        // Retrieve available extensions into a vector
        std::vector<VkExtensionProperties> extensions(extensionCount);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to enumerate instance extension properties");
        }

        // Collect available extensions into a set for fast lookup
        std::unordered_set<std::string> availableExtensions;
        std::cout << "Available extensions:" << std::endl;
        for (const auto& extension : extensions) {
            std::string extensionName(extension.extensionName);
            availableExtensions.insert(extensionName);
            std::cout << "\t [ " << std::setw(40) << std::left << extensionName << " ]" << std::endl;
        }

        // Check if all required extensions are present
        auto requiredExtensions = getInstanceExtensions();
        std::cout << "Required extensions:" << std::endl;
        bool allRequiredExtensionsPresent = true;

        for (const auto& requiredExtension : requiredExtensions) {
            std::cout << "\t [ " << std::setw(40) << std::left << requiredExtension << " ]" << std::endl;
            if (availableExtensions.find(requiredExtension) == availableExtensions.end()) {
                std::cerr << "Missing required extension: " << requiredExtension << std::endl;
                allRequiredExtensionsPresent = false;
            }
        }

        if (!allRequiredExtensionsPresent) {
            throw std::runtime_error("Failed to find one or more required extensions");
        }
    }

    /**
     * @brief Retrieves the necessary instance extensions, including debug utilities if validation layers are enabled.
     *
     * @return A vector of required instance extension names.
     */
    std::vector<const char*> Device::getInstanceExtensions() {
        std::vector<const char*> extensions = m_Window.getExtensions();

        // If validation layers are enabled AND available, add the debug utils extension
        if (m_isValidationLayerON && m_areValidationLayersAvailable) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef __APPLE__
            // Add additional extensions required for Apple platforms
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
        }

        return extensions;
    }

    /**
     * @brief Callback function for handling validation layer messages, formatted for readability.
     *
     * @param messageSeverity The severity of the validation layer message.
     * @param messageType The type of message (general, validation, performance).
     * @param pCallbackData Information about the message.
     * @param pUserData User data passed to the callback function.
     *
     * @return VK_FALSE to indicate that the message has been handled.
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << std::setfill('<') << std::setw(60) << "\n <<<<<<<<<<<<<<<< VALIDATION LAYER MESSAGE START " << std::endl;
        std::cerr << std::setfill(' ') << std::setw(4) << " " << std::left << pCallbackData->pMessage << std::endl;
        std::cerr << std::setfill('>') << std::setw(60) << " >>>>>>>>>>>>>>>> VALIDATION LAYER MESSAGE END " << std::endl;
        return VK_FALSE;
    }

    /**
     * @brief Enables debugging capabilities by setting up the debug messenger for validation layers.
     *
     * @return VK_SUCCESS on success, VK_ERROR_EXTENSION_NOT_PRESENT if the debug extension is not available.
     *
     * @throws std::runtime_error If the debug messenger creation fails.
     */
    VkResult Device::enableDebuggingCapabilities() {
        if (!m_isValidationLayerON || !m_areValidationLayersAvailable) return VK_SUCCESS;  // No debugging if validation layers are disabled or unavailable

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugUtilsMessageCallback;  // Callback for debug messages
        createInfo.pUserData = nullptr;

        // Retrieve function pointers for debug messenger creation and destruction
        m_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
        if (m_vkCreateDebugUtilsMessengerEXT != nullptr) {
            m_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
            if (m_vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("Debug messenger creation failed!");
            }
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        return VK_SUCCESS;
    }

    /**
     * @brief Creates the Vulkan surface associated with the window for rendering.
     */
    void Device::createWindowSurface() {
        m_Window.createWindowSurface(m_Instance, &m_SurfaceKHR);
    }

    /**
     * @brief Enumerates physical devices (GPUs) and selects one based on required features and extensions.
     *
     * @throws std::runtime_error If no suitable physical device (GPU) is found.
     */
    void Device::enumerateAndSelectPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        std::cout << "Device count: " << deviceCount << std::endl;

        // Retrieve and evaluate all available physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        // Select the first device that meets the requirements
        for (const auto& device : devices) {
            DeviceHelper::QueueFamilyIndices queueFamilyIndices;
            queueFamilyIndices.graphicsFamily = DeviceHelper::queryQueueFamilies(device, VK_QUEUE_GRAPHICS_BIT, m_SurfaceKHR);

            // Check if the device supports the required extensions
            bool extensionsSupported = checkDeviceExtensionSupport(device);

            // Check swapchain support if extensions are supported
            bool swapChainAdequate = false;
            if (extensionsSupported) {
                SwapChainHelper::SwapChainMetaData swapChainSupport = SwapChainHelper::swapChainMetadata(device, m_SurfaceKHR);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            // Check for required features (e.g., anisotropic filtering) and select the device
            if (extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy) {
                m_PhysicalDevice = device;
                break;
            }
        }

        // If no suitable device is found, throw an error
        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);  // Retrieve device properties
        std::cout << "Physical device: " << properties.deviceName << std::endl;
    }

    /**
     * @brief Creates the logical device and retrieves the necessary queues (graphics, compute, transfer).
     *
     * @throws std::runtime_error If logical device creation fails.
     */
    void Device::createLogicalDeviceAndConfigureQueues() {
        DeviceHelper::QueueFamilyIndices queueFamilyIndices;
        queueFamilyIndices.graphicsFamily = DeviceHelper::queryQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT, m_SurfaceKHR);
        queueFamilyIndices.computeFamily = DeviceHelper::queryQueueFamilies(m_PhysicalDevice, VK_QUEUE_COMPUTE_BIT);
        queueFamilyIndices.transferFamily = DeviceHelper::queryQueueFamilies(m_PhysicalDevice, VK_QUEUE_TRANSFER_BIT);

        // Prepare queue creation info based on unique queue families
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Enable the necessary device features
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        // Create the logical device
        VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<uint32_t>(m_RequestedDeviceExtensions.size()),
        .ppEnabledExtensionNames = m_RequestedDeviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
        };

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS) {
            throw std::runtime_error("Logical device creation failed!");
        }

        // Retrieve the graphics queue handle
        vkGetDeviceQueue(m_LogicalDevice, queueFamilyIndices.graphicsFamily, 0, &m_GraphicsQueue);
    }
    /**
     * @brief Creates a command pool for allocating and managing command buffers.
     *
     * The command pool allows command buffers to be reset and is set to transient for short-lived buffers.
     *
     * @throws std::runtime_error If the command pool creation fails.
     */
    void Device::createCommandPool() {
        VkCommandPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  // Allow command buffers to be reset and short-lived
        .queueFamilyIndex = DeviceHelper::queryQueueFamilies(m_PhysicalDevice, VK_QUEUE_GRAPHICS_BIT, m_SurfaceKHR),
        };

        // Create the command pool and check for errors
        if (vkCreateCommandPool(m_LogicalDevice, &poolCreateInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Command pool creation failed!");
        }
    }

    /**
     * @brief Queries for queue families on the physical device that match the desired flags (graphics, compute, transfer).
     *
     * @param device The Vulkan physical device to query.
     * @param queryQueueFlags The queue flags to search for (e.g., VK_QUEUE_GRAPHICS_BIT).
     * @param surfaceKHR Optional, the Vulkan surface handle (required for presentation support).
     *
     * @return The index of the queue family that matches the query flags, or UINT32_MAX if no suitable queue family is found.
     */
    uint32_t DeviceHelper::queryQueueFamilies(VkPhysicalDevice device, VkQueueFlags queryQueueFlags, VkSurfaceKHR surfaceKHR) {
        uint32_t requestedQueueIndex = UINT32_MAX;

        // Retrieve the number of queue families supported by the device
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // Get the properties of each queue family
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

        // Iterate over all queue families to find one that meets the required queue flags
        for (uint32_t index = 0; index < queueFamilyCount; ++index) {
            const VkQueueFamilyProperties& queueFamily = queueFamilyProperties[index];

            if (queueFamily.queueCount <= 0) continue;

            // Check if the queue family supports graphics operations
            bool isGraphicsQueueRequest = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                && ((queryQueueFlags & VK_QUEUE_GRAPHICS_BIT) == queryQueueFlags);

            // If it's a graphics queue request, check if it supports the surface (for presentation)
            if (isGraphicsQueueRequest) {
                ASSERT_WITH_MSG(surfaceKHR != VK_NULL_HANDLE, "Did you forget to pass KhronosKHR handle");

                requestedQueueIndex = index;

                // Verify if the queue also supports presentation (for rendering to the screen)
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surfaceKHR, &presentSupport);
                if (presentSupport) {
                    break;  // Stop searching once a queue with both graphics and presentation support is found
                }
            }

            // Check if it's a dedicated compute queue (must not support graphics)
            bool isComputeQueueRequest = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                && ((queryQueueFlags & VK_QUEUE_COMPUTE_BIT) == queryQueueFlags)
                && ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0);
            if (isComputeQueueRequest) {
                requestedQueueIndex = index;
            }

            // Check if it's a transfer queue
            bool isTransferQueueRequest = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                && ((queryQueueFlags & VK_QUEUE_TRANSFER_BIT) == queryQueueFlags);
            if (isTransferQueueRequest) {
                requestedQueueIndex = index;
            }

            // If any other queue matches the desired queue flags, select it
            if ((queueFamily.queueFlags & queryQueueFlags) == queryQueueFlags) {
                requestedQueueIndex = index;
            }
        }

        return requestedQueueIndex;
    }

    /**
     * @brief Checks if the physical device supports the required extensions.
     *
     * @param device The Vulkan physical device to query.
     *
     * @return True if all required extensions are supported; otherwise, false.
     */
    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // Convert required extensions to a set for easy comparison
        std::set<std::string> requiredExtensions(m_RequestedDeviceExtensions.begin(), m_RequestedDeviceExtensions.end());

        // Remove supported extensions from the required extensions set
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        // Return true if all required extensions are supported (set should be empty)
        return requiredExtensions.empty();
    }

    /**
     * @brief Begins a new command buffer and returns it for recording commands.
     *
     * @return The allocated and initialized command buffer ready for recording commands.
     */
    VkCommandBuffer Device::beginCommandBuffer() {
        // Command buffer allocation information
        VkCommandBufferAllocateInfo cmdBufAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
        };

        VkCommandBuffer cmdBuffer;
        VkResult result = vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufAllocInfo, &cmdBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffer.");
        }

        // Command buffer begin information
        VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // Buffer is used only once

        result = vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin command buffer.");
        }

        return cmdBuffer;
    }

    /**
     * @brief Ends the recording of a command buffer and submits it to the graphics queue.
     *
     * @param commandBuffer The command buffer that has finished recording commands.
     */
    void Device::endCommandBuffer(VkCommandBuffer commandBuffer) {
        // End the command buffer recording
        VkResult result = vkEndCommandBuffer(commandBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to end command buffer recording.");
        }

        // Submit the command buffer to the graphics queue
        VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        };

        result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit command buffer.");
        }

        // Wait until all operations on the queue are finished
        result = vkQueueWaitIdle(m_GraphicsQueue);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to wait for queue idle.");
        }

        // Free the command buffer after execution
        vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
    }

    /**
     * @brief Finds a suitable memory type from the device's available memory types.
     *
     * @param physicalDevice The Vulkan physical device to query.
     * @param memType A bitmask of memory types that are acceptable.
     * @param propertyFlags The required memory property flags (e.g., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT).
     *
     * @return The index of a suitable memory type.
     *
     * @throws std::runtime_error If no suitable memory type is found.
     */
    uint32_t DeviceHelper::getMemoryType(VkPhysicalDevice physicalDevice, uint32_t memType, VkMemoryPropertyFlags propertyFlags) {
        VkPhysicalDeviceMemoryProperties physicalDevMemProp;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDevMemProp);

        // Loop through all available memory types and find one that matches the type filter and property flags
        for (uint32_t i = 0; i < physicalDevMemProp.memoryTypeCount; i++) {
            auto isMemoryTypeAvailable = (memType & (1 << i));
            auto propertyFlagBit = (physicalDevMemProp.memoryTypes[i].propertyFlags & propertyFlags);
            if (isMemoryTypeAvailable && (propertyFlagBit == propertyFlags)) {
                return i;
            }
        }

        throw std::runtime_error("Unable to locate an appropriate memory type.");
    }
}  // namespace giraphics
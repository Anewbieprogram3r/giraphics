#include <Engine/Graphics/Vulkan/Device.hpp>
#include <Engine/Graphics/Window.hpp>

namespace giraphics {
    /**
     * @brief Constructs the Device object and initializes Vulkan instance, device, and other necessary components.
     *
     * @param window Reference to the Window object used to create the Vulkan surface.
     */
    Device::Device(Window& window) : m_Window{ window } {
        m_areValidationLayersAvailable = isRequestedValidationLayersAvailable();
        createVulkanInstance();
        enumerateInstanceExtensions();
        enableDebuggingCapabilities();
    }

    /**
     * @brief Destroys the Vulkan device and cleans up all associated resources.
     *
     * This includes destroying the command pool, logical device, debug messenger (if validation layers are enabled),
     * Vulkan surface, and Vulkan instance.
     */
    Device::~Device() {
        if (m_isValidationLayerON && m_vkDestroyDebugUtilsMessengerEXT) {
            m_vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

    bool Device::isRequestedValidationLayersAvailable() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Loop through each requested validation layer and check if it exists
        for (const char* layerName : m_RequestedValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
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

    void Device::createVulkanInstance() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "My Vulkan App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef __APPLE__
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        createInfo.pApplicationInfo = &appInfo;
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
}  // namespace giraphics
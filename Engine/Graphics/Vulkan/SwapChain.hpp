#pragma once

#include <Engine/Graphics/Vulkan/Device.hpp>

namespace giraphics {

    // SwapChain class manages Vulkan's swap chain, which handles rendering images to the screen.
    // It is a non-copyable class, preventing copy constructor and assignment for resource safety.
    class SwapChain : public NonCopyable {
    public:
        // Maximum number of frames that can be in flight at once.
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        // Constructor: Initializes the swap chain with a reference to a Vulkan device and window extent.
        SwapChain(Device& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous = nullptr);

        // Destructor: Cleans up the swap chain resources.
        ~SwapChain();

        // Accessor for framebuffer by index.
        VkFramebuffer frameBuffer(int index) { return m_SwapChainFramebuffers[index]; }

        // Accessor for the render pass associated with the swap chain.
        VkRenderPass renderPass() const { return m_RenderPass; }

        // Accessor for image view by index.
        VkImageView imageView(int index) { return m_SwapChainImageViews[index]; }

        // Get the total number of swap chain images.
        size_t swapChainImageCount() { return m_SwapChainImages.size(); }

        // Get the image format of the swap chain.
        VkFormat swapChainImageFormat() const { return m_SwapChainImageFormat; }

        // Get the extent (width and height) of the swap chain.
        VkExtent2D swapChainExtent() const { return m_SwapChainExtent; }

        // Convenience function to get the width of the swap chain.
        uint32_t width() const { return m_SwapChainExtent.width; }

        // Convenience function to get the height of the swap chain.
        uint32_t height() const { return m_SwapChainExtent.height; }

        // Calculate and return the aspect ratio (width/height) of the swap chain.
        float extentAspectRatio() const {
            return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
        }

        // Find and return a suitable depth format for the swap chain.
        VkFormat findSuitableDepthFormat();

        // Acquire the next image from the swap chain, storing the image index in the provided pointer.
        VkResult acquireNextImage(uint32_t* imageIndex);

        // Submit command buffers for rendering to the swap chain.
        VkResult enqueueCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        // Create the swap chain resources. Optionally takes the old swap chain to reuse resources.
        void createSwapChain(std::shared_ptr<SwapChain> oldSwapchain);

        // Create the color buffer images (image views) for the swap chain.
        void createColorBufferImages();

        // Create the depth buffer images for the swap chain.
        void createDepthBufferImages();

        // Create the render pass used by the swap chain.
        void createRenderPass();

        // Create the framebuffers for the swap chain images.
        void setupFramebuffer();

        // Create Vulkan synchronization objects (e.g., semaphores and fences) for frame rendering.
        void setupSynchronizationObjects();

        // Helper function for creating an image, with specified creation info and memory properties.
        void createImage(
            const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);

        // Helper function to select a non-linear sRGB surface format from the available formats.
        VkSurfaceFormatKHR srgbNonLinearFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        // Helper function to select the preferred presentation mode (e.g., FIFO, Mailbox) from available modes.
        VkPresentModeKHR prefferedPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        // Helper function to choose the swap extent (resolution) based on surface capabilities.
        VkExtent2D extent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        // Swap chain's image format (color format) and depth format.
        VkFormat m_SwapChainImageFormat;
        VkFormat m_SwapChainDepthFormat;

        // Swap chain's extent (resolution).
        VkExtent2D m_SwapChainExtent;

        // Framebuffers used by the swap chain, corresponding to the swap chain images.
        std::vector<VkFramebuffer> m_SwapChainFramebuffers;

        // Vulkan render pass used by the swap chain.
        VkRenderPass m_RenderPass;

        // Depth images and associated memory and image views.
        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemories;
        std::vector<VkImageView> m_DepthImageViews;

        // Swap chain images and associated image views.
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        // Reference to the device (GPU) associated with this swap chain.
        Device& m_Device;

        // Window extent (resolution) for the swap chain.
        VkExtent2D m_WindowExtent;

        // Vulkan swap chain handle.
        VkSwapchainKHR m_SwapChain;

        struct FrameSyncObjects {
            std::vector<VkSemaphore> imageReadySemaphores;
            std::vector<VkSemaphore> presentReadySemaphores;
            std::vector<VkFence> activeFrameFences;
            std::vector<VkFence> swapchainImageFences;

            FrameSyncObjects(VkDevice device, int maxFrameInFlight, size_t swapChainImageCount);
            ~FrameSyncObjects();
            VkDevice m_Device;
        };

        std::unique_ptr<FrameSyncObjects> m_SyncObjects;

        // Track the current frame being rendered (for managing frame synchronization).
        size_t m_CurrentFrame = 0;

        // Declare friend classes that can access private members of this class.
        friend class ImguiManager;
        friend class Renderer;
    };

    // Helper struct for querying swap chain support information from the Vulkan physical device and surface.
    struct SwapChainHelper {
        struct SwapChainMetaData {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        // Function to get swap chain support details for a physical device and surface.
        static SwapChainMetaData swapChainMetadata(VkPhysicalDevice m_PhysicalDevice, VkSurfaceKHR m_SurfaceKHR);
        static VkSemaphore createSemaphore(VkDevice device);
        static VkFence createFence(VkDevice device, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);
    };
}  // namespace giraphics
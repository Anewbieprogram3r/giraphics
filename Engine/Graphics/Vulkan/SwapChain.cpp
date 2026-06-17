#include <Engine/Graphics/Vulkan/SwapChain.hpp>

namespace giraphics {

    /**
     * @brief Constructs a new SwapChain object by initializing its resources.
     *
     * @param device The Vulkan device for creating and managing the swapchain.
     * @param extent The window extent (width and height) used to determine the size of the swapchain.
     * @param oldSwapchain A shared pointer to the previous swapchain (for resource reuse).
     */
    SwapChain::SwapChain(Device& device, VkExtent2D extent, std::shared_ptr<SwapChain> oldSwapchain)
        : m_Device{ device },
        m_WindowExtent{ extent } {
    }

    /**
     * @brief Destructor for SwapChain, ensures proper resource cleanup.
     *
     * Destroys framebuffers, depth buffers, render pass, image views, and the swapchain itself.
     */
    SwapChain::~SwapChain() {
        // Destroy framebuffers
        for (auto framebuffer : m_SwapChainFramebuffers) {
            if (framebuffer) {
                vkDestroyFramebuffer(m_Device.device(), framebuffer, nullptr);
            }
        }

        // Destroy depth buffers
        for (size_t i = 0; i < m_DepthImages.size(); ++i) {
            if (m_DepthImageViews[i]) {
                vkDestroyImageView(m_Device.device(), m_DepthImageViews[i], nullptr);
            }
            if (m_DepthImages[i]) {
                vkDestroyImage(m_Device.device(), m_DepthImages[i], nullptr);
            }
            if (m_DepthImageMemories[i]) {
                vkFreeMemory(m_Device.device(), m_DepthImageMemories[i], nullptr);
            }
        }

        // Destroy render pass
        if (m_RenderPass) {
            vkDestroyRenderPass(m_Device.device(), m_RenderPass, nullptr);
        }

        // Destroy image views
        for (auto imageView : m_SwapChainImageViews) {
            if (imageView) {
                vkDestroyImageView(m_Device.device(), imageView, nullptr);
            }
        }

        // Destroy swapchain
        if (m_SwapChain) {
            vkDestroySwapchainKHR(m_Device.device(), m_SwapChain, nullptr);
            m_SwapChain = nullptr;
        }
    }

    /**
     * @brief Creates a Vulkan swapchain for presenting images to the window surface.
     *
     * Sets up and creates a new swapchain with the appropriate configurations, including image formats,
     * presentation modes, and image counts.
     *
     * @param oldSwapchain A shared pointer to the previous swapchain to enable a seamless transition.
     *
     * @throws std::runtime_error if the swapchain creation fails.
     */
    void SwapChain::createSwapChain(std::shared_ptr<SwapChain> oldSwapchain) {
        // Retrieve metadata about the swapchain support for the current physical device and surface.
        SwapChainHelper::SwapChainMetaData swapChainMetadata = SwapChainHelper::swapChainMetadata(m_Device.m_PhysicalDevice, m_Device.m_SurfaceKHR);

        // Choose a suitable surface format (sRGB format with non-linear color space).
        VkSurfaceFormatKHR surfaceFormat = srgbNonLinearFormat(swapChainMetadata.formats);
        if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {
            throw std::runtime_error("No suitable surface format found.");
        }

        // Select a presentation mode (prefer vsync to avoid screen tearing).
        VkPresentModeKHR presentMode = prefferedPresentMode(swapChainMetadata.presentModes);
        if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR) {
            throw std::runtime_error("No suitable present mode found.");
        }

        // Determine the size (extent) of the swapchain images to match the window's dimensions.
        VkExtent2D dimension = extent(swapChainMetadata.capabilities);
        if (dimension.width == 0 || dimension.height == 0) {
            throw std::runtime_error("Invalid swap chain extent.");
        }

        // Calculate the number of swapchain images (min + 1 for smooth rendering).
        uint32_t imageCount = swapChainMetadata.capabilities.minImageCount + 1;
        if (swapChainMetadata.capabilities.maxImageCount > 0 && imageCount > swapChainMetadata.capabilities.maxImageCount) {
            imageCount = swapChainMetadata.capabilities.maxImageCount;
        }

        // Set up the swapchain creation info.
        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_Device.surface(),
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = dimension,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = swapChainMetadata.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain ? oldSwapchain->m_SwapChain : VK_NULL_HANDLE
        };

        // Create the swapchain.
        VkResult result = vkCreateSwapchainKHR(m_Device.device(), &createInfo, nullptr, &m_SwapChain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain. VkResult: " + std::to_string(result));
        }

        // Retrieve the swapchain images (actual count may differ from requested).
        uint32_t actualImageCount = 0;
        result = vkGetSwapchainImagesKHR(m_Device.device(), m_SwapChain, &actualImageCount, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to retrieve swap chain images. VkResult: " + std::to_string(result));
        }

        m_SwapChainImages.resize(actualImageCount);
        result = vkGetSwapchainImagesKHR(m_Device.device(), m_SwapChain, &actualImageCount, m_SwapChainImages.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to retrieve swap chain images (second call). VkResult: " + std::to_string(result));
        }

        // Store the format and extent of the swapchain images for future use.
        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = dimension;
    }

    /**
     * @brief Creates image views for the color buffer images in the swapchain.
     *
     * Loops through each swapchain image and creates a corresponding image view, which allows the
     * images to be used as color attachments during rendering.
     *
     * @throws std::runtime_error if creating an image view fails.
     */
    void SwapChain::createColorBufferImages() {
        // Resize the vector to match the number of swapchain images
        m_SwapChainImageViews.resize(m_SwapChainImages.size());

        // Loop through each swapchain image to create a corresponding image view
        for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_SwapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_SwapChainImageFormat,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image view for color buffer!");
            }
        }
    }

    /**
     * @brief Retrieves the swapchain image extent based on surface capabilities and the current window size.
     *
     * @param capabilities The surface capabilities (min/max image extent, current extent, etc.).
     * @return VkExtent2D The calculated or current swapchain image extent.
     */
    VkExtent2D SwapChain::extent(const VkSurfaceCapabilitiesKHR& capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            VkExtent2D actualExtent = m_WindowExtent;

            if (actualExtent.width == 0 || actualExtent.height == 0) {
                actualExtent.width = capabilities.minImageExtent.width;
                actualExtent.height = capabilities.minImageExtent.height;
            }

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    /**
     * @brief Selects an sRGB non-linear color space format from the list of supported formats.
     *
     * @param supportedFormats A list of supported surface formats.
     * @return VkSurfaceFormatKHR The selected surface format.
     */
    VkSurfaceFormatKHR SwapChain::srgbNonLinearFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats) {
        for (const auto& availableFormat : supportedFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return supportedFormats.empty() ? VkSurfaceFormatKHR{} : supportedFormats[0];
    }

    /**
     * @brief Selects the preferred present mode for the swapchain from the available options.
     *
     * Falls back to FIFO (vsync) if preferred modes (MAILBOX, IMMEDIATE) are unavailable.
     *
     * @param availablePresentModes A list of available present modes supported by the swapchain.
     * @return VkPresentModeKHR The selected present mode.
     */
    VkPresentModeKHR SwapChain::prefferedPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        const std::vector<VkPresentModeKHR> preferredModes = {
            VK_PRESENT_MODE_MAILBOX_KHR,
            VK_PRESENT_MODE_IMMEDIATE_KHR
        };

        for (const auto& preferredMode : preferredModes) {
            if (std::find(availablePresentModes.begin(), availablePresentModes.end(), preferredMode) != availablePresentModes.end()) {
                std::cout << "Current Swap Chain Present Mode: "
                    << (preferredMode == VK_PRESENT_MODE_MAILBOX_KHR ? "Mailbox" : "Immediate")
                    << std::endl;
                return preferredMode;
            }
        }

        std::cout << "Preferred MailBox Present Mode unavailable, falling back to FIFO Mode" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    /**
     * @brief Creates depth buffer images and image views for each swapchain image.
     *
     * Creates a depth image and view per swapchain image for depth testing during rendering.
     *
     * @throws std::runtime_error if creating a depth image view fails.
     */
    void SwapChain::createDepthBufferImages() {
        VkFormat depthFormat = findSuitableDepthFormat();
        m_SwapChainDepthFormat = depthFormat;

        VkExtent2D swapChainExt = swapChainExtent();
        auto imageCount = swapChainImageCount();
        m_DepthImages.resize(imageCount);
        m_DepthImageMemories.resize(imageCount);
        m_DepthImageViews.resize(imageCount);

        for (int i = 0; i < m_DepthImages.size(); i++) {
            VkImageCreateInfo imageInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = depthFormat,
                .extent = {swapChainExt.width, swapChainExt.height, 1},
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
            };

            createImage(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_DepthImages[i],
                m_DepthImageMemories[i]);

            VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_DepthImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = depthFormat,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_DepthImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image view for depth buffer!");
            }
        }
    }

    /**
     * @brief Creates a Vulkan render pass for the swapchain.
     *
     * Defines color and depth attachments, one subpass, and subpass dependencies.
     *
     * @throws std::runtime_error if render pass creation fails.
     */
    void SwapChain::createRenderPass() {
        VkAttachmentDescription depthAttachment{
            .flags = 0,
            .format = findSuitableDepthFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depthAttachmentRef{
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentDescription colorAttachment{
            .format = swapChainImageFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
        };

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0
        };

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };

        if (vkCreateRenderPass(m_Device.device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    /**
     * @brief Finds a suitable depth format supported by the physical device.
     *
     * @return VkFormat The selected depth format.
     * @throws std::runtime_error if no suitable depth format is found.
     */
    VkFormat SwapChain::findSuitableDepthFormat() {
        const std::vector<VkFormat> depthFormats = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

        for (VkFormat format : depthFormats) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_Device.m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("No suitable depth format found!");
    }

    /**
     * @brief Creates a Vulkan image and allocates memory for it.
     *
     * @param imageInfo The image creation information.
     * @param properties Memory property flags.
     * @param image Reference to the Vulkan image to be created.
     * @param imageMemory Reference to the Vulkan memory object to be allocated.
     * @throws std::runtime_error if image creation, memory allocation, or binding fails.
     */
    void SwapChain::createImage(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        if (vkCreateImage(m_Device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("Image creation failed!");
        }

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_Device.device(), image, &memoryRequirements);

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memoryRequirements.size;
        memAllocInfo.memoryTypeIndex = DeviceHelper::getMemoryType(
            m_Device.m_PhysicalDevice,
            memoryRequirements.memoryTypeBits,
            properties
        );

        if (vkAllocateMemory(m_Device.device(), &memAllocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("Could not allocate memory for image.");
        }

        if (vkBindImageMemory(m_Device.device(), image, imageMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("Image memory binding unsuccessful");
        }
    }

    /**
     * @brief Creates framebuffers for each swapchain image.
     *
     * Combines color and depth image views into framebuffers used during rendering.
     *
     * @throws std::runtime_error if framebuffer creation fails.
     */
    void SwapChain::setupFramebuffer() {
        auto imageCount = swapChainImageCount();
        m_SwapChainFramebuffers.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            std::array<VkImageView, 2> attachments = { m_SwapChainImageViews[i], m_DepthImageViews[i] };

            VkExtent2D swapChainExt = swapChainExtent();

            VkFramebufferCreateInfo framebufferInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = swapChainExt.width,
                .height = swapChainExt.height,
                .layers = 1,
            };

            if (vkCreateFramebuffer(m_Device.device(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Framebuffer object creation failed!");
            }
        }
    }

    /**
     * @brief Creates synchronization objects required for rendering and presentation.
     *
     * Initializes semaphores and fences for synchronizing frame acquisition and presentation.
     */
    void SwapChain::setupSynchronizationObjects() {
        m_SyncObjects = std::make_unique<FrameSyncObjects>(m_Device.device(), MAX_FRAMES_IN_FLIGHT, swapChainImageCount());
    }

    /**
     * @brief Acquires the next image from the swapchain.
     *
     * Waits on the current frame fence, then acquires the next available swapchain image.
     *
     * @param imageIndex Pointer to store the index of the acquired image.
     * @return VkResult Result of the image acquisition.
     * @throws std::runtime_error if waiting for the fence fails.
     */
    VkResult SwapChain::acquireNextImage(uint32_t* imageIndex) {
        VkResult fenceResult = vkWaitForFences(
            m_Device.device(),
            1,
            &m_SyncObjects->activeFrameFences[m_CurrentFrame],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max()
        );

        if (fenceResult != VK_SUCCESS) {
            throw std::runtime_error("Failed to wait for fences. VkResult: " + std::to_string(fenceResult));
        }

        VkResult result = vkAcquireNextImageKHR(
            m_Device.device(),
            m_SwapChain,
            std::numeric_limits<uint64_t>::max(),
            m_SyncObjects->imageReadySemaphores[m_CurrentFrame],
            VK_NULL_HANDLE,
            imageIndex
        );

        return result;
    }

    /**
     * @brief Submits command buffers for execution and presents the rendered image to the swapchain.
     *
     * Manages synchronization via semaphores and fences, submits the command buffer, then presents the image.
     *
     * @param buffers Pointer to the command buffers to be submitted.
     * @param imageIndex Pointer to the index of the image being rendered.
     * @return VkResult Result of the presentation operation.
     * @throws std::runtime_error if submitting or presenting fails.
     */
    VkResult SwapChain::enqueueCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
        if (m_SyncObjects->swapchainImageFences[*imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(
                m_Device.device(),
                1,
                &m_SyncObjects->swapchainImageFences[*imageIndex],
                VK_TRUE,
                UINT64_MAX
            );
        }

        m_SyncObjects->swapchainImageFences[*imageIndex] = m_SyncObjects->activeFrameFences[m_CurrentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_SyncObjects->imageReadySemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = { m_SyncObjects->presentReadySemaphores[*imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_Device.device(), 1, &m_SyncObjects->activeFrameFences[m_CurrentFrame]);

        if (vkQueueSubmit(m_Device.queue(), 1, &submitInfo, m_SyncObjects->activeFrameFences[m_CurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_SwapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = imageIndex;

        VkResult result = vkQueuePresentKHR(m_Device.queue(), &presentInfo);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    /**
     * @brief Constructs FrameSyncObjects, creating semaphores and fences for each frame in flight.
     */
    SwapChain::FrameSyncObjects::FrameSyncObjects(VkDevice device, int maxFrameInFlight, size_t swapChainImageCount)
        : m_Device(device) {
        imageReadySemaphores.resize(maxFrameInFlight);
        presentReadySemaphores.resize(swapChainImageCount);
        activeFrameFences.resize(maxFrameInFlight);
        swapchainImageFences.resize(swapChainImageCount, VK_NULL_HANDLE);

        for (size_t i = 0; i < maxFrameInFlight; ++i) {
            imageReadySemaphores[i] = SwapChainHelper::createSemaphore(device);
            activeFrameFences[i] = SwapChainHelper::createFence(device);
        }
        for (size_t i = 0; i < swapChainImageCount; ++i) {
            presentReadySemaphores[i] = SwapChainHelper::createSemaphore(device);
        }
    }

    /**
     * @brief Destructor for FrameSyncObjects, destroys all semaphores and fences.
     */
    SwapChain::FrameSyncObjects::~FrameSyncObjects() {
        for (size_t i = 0; i < presentReadySemaphores.size(); ++i) {
            if (presentReadySemaphores[i]) {
                vkDestroySemaphore(m_Device, presentReadySemaphores[i], nullptr);
            }
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            if (imageReadySemaphores[i]) {
                vkDestroySemaphore(m_Device, imageReadySemaphores[i], nullptr);
            }
            if (activeFrameFences[i]) {
                vkDestroyFence(m_Device, activeFrameFences[i], nullptr);
            }
        }
    }

    /**
     * @brief Retrieves metadata about the swapchain support for the given physical device and surface.
     *
     * This function gathers information about the surface's capabilities, supported formats, and available present modes
     * for the swapchain. It queries the physical device and the surface to populate the `SwapChainMetaData` structure
     * with relevant details needed for swapchain creation.
     *
     * @param physicalDevice The Vulkan physical device to query for swapchain support.
     * @param surface The surface associated with the swapchain.
     * @return SwapChainHelper::SwapChainMetaData A structure containing surface capabilities, supported formats, and present modes.
     *
     * @throws std::runtime_error if any Vulkan query for surface capabilities, formats, or present modes fails.
     */
    SwapChainHelper::SwapChainMetaData SwapChainHelper::swapChainMetadata(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        SwapChainMetaData swapChainMetadata;

        // Get surface capabilities (image count, extent, etc.)
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainMetadata.capabilities);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to get surface capabilities.");
        }

        // Get the number of supported surface formats
        uint32_t formatCount = 0;
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to query surface format count.");
        }

        // If there are supported formats, retrieve them
        if (formatCount > 0) {
            swapChainMetadata.formats.resize(formatCount);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapChainMetadata.formats.data());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to get surface formats.");
            }
        }

        // Get the number of supported present modes
        uint32_t presentModeCount = 0;
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to query present mode count.");
        }

        // If there are present modes, retrieve them
        if (presentModeCount > 0) {
            swapChainMetadata.presentModes.resize(presentModeCount);
            result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapChainMetadata.presentModes.data());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to get surface present modes.");
            }
        }

        return swapChainMetadata;  // Return the collected metadata
    }

    /**
     * @brief Creates a Vulkan semaphore for GPU-GPU synchronization.
     *
     * @param device The Vulkan logical device.
     * @return VkSemaphore The created semaphore handle.
     * @throws std::runtime_error if semaphore creation fails.
     */
    VkSemaphore SwapChainHelper::createSemaphore(VkDevice device) {
        VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkSemaphore semaphore;
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphore!");
        }

        return semaphore;
    }

    /**
     * @brief Creates a Vulkan fence for CPU-GPU synchronization.
     *
     * @param device The Vulkan logical device.
     * @param flags Optional flags (e.g., VK_FENCE_CREATE_SIGNALED_BIT).
     * @return VkFence The created fence handle.
     * @throws std::runtime_error if fence creation fails.
     */
    VkFence SwapChainHelper::createFence(VkDevice device, VkFenceCreateFlags flags) {
        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = flags
        };

        VkFence fence;
        if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence!");
        }

        return fence;
    }

}  // namespace giraphics
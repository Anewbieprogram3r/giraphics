#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <Engine/Graphics/Utils/Utils.hpp>

namespace giraphics {
    class Input;
    class GlfwWindow : public NonCopyable {
    public:
        GlfwWindow(int w, int h, std::string name);
        ~GlfwWindow();

        bool shouldClose() { return glfwWindowShouldClose(m_GlfwWindow); }
        void pollEvents() { return glfwPollEvents(); }

        void waitForEvents();
        uint32_t getWidth() const {
            return static_cast<uint32_t>(m_Width);
        }
        uint32_t getHeight() const {
            return static_cast<uint32_t>(m_Height);
        }

        std::vector<const char*> getExtensions();

        bool wasWindowResized() { return m_IsWindowResized; }
        void resetWindowResizedFlag() { m_IsWindowResized = false; }
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        void setInput(Input& input);
        void* glfwWindow();

        Input* input() const {
            return m_Input;
        }

    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        void setupWindow();

        int m_Width = 800;
        int m_Height = 600;
        bool m_IsWindowResized = false;

        std::string m_WindowName;
        Input* m_Input = nullptr;
        GLFWwindow* m_GlfwWindow;
    };
}  // namespace giraphics
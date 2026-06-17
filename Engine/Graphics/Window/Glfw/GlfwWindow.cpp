#include <Engine/Graphics/Window/Glfw/GlfwWindow.hpp>

namespace giraphics {
    GlfwWindow::GlfwWindow(int w, int h, std::string name) : m_Width{ w }, m_Height{ h }, m_WindowName{ name } {
        setupWindow();
    }

    GlfwWindow::~GlfwWindow() {
        glfwDestroyWindow(m_GlfwWindow);
        glfwTerminate();
    }

    void GlfwWindow::waitForEvents()
    {
        glfwWaitEvents();
    }

    void GlfwWindow::setupWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_GlfwWindow = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_GlfwWindow, this);
        glfwSetFramebufferSizeCallback(m_GlfwWindow, framebufferResizeCallback);
    }

    std::vector<const char*> GlfwWindow::getExtensions()
    {
        uint32_t count = 0;
        const char** extensions;
        extensions = glfwGetRequiredInstanceExtensions(&count);

        return std::vector<const char*>(extensions, extensions + count);
    }

    void GlfwWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, m_GlfwWindow, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void GlfwWindow::setInput(Input& input)
    {
        m_Input = &input;
    }

    void* GlfwWindow::glfwWindow()
    {
        return m_GlfwWindow;
    }

    void GlfwWindow::framebufferResizeCallback(GLFWwindow* p_window, int p_width, int p_height) {
        auto window = reinterpret_cast<GlfwWindow*>(glfwGetWindowUserPointer(p_window));
        window->m_IsWindowResized = true;
        window->m_Width = p_width;
        window->m_Height = p_height;
    }

}  // namespace giraphics
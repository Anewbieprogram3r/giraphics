#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Graphics/Window/Window.hpp"

namespace giraphics {

class GlfwWindow : public Window {
public:
    GlfwWindow(const std::string& title, int width, int height);
    ~GlfwWindow() override;

    bool shouldClose() const override;
    void pollEvents()  const override;

    GLFWwindow* nativeHandle() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
};

} // namespace giraphics
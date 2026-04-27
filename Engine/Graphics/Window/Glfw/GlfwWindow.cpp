#include "GlfwWindow.hpp"
#include <stdexcept>
#include <iostream>

namespace giraphics {

GlfwWindow::GlfwWindow(const std::string& title, int width, int height)
    : Window(title, width, height)
{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialise GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    std::cout << "[GlfwWindow] Window created: " << title
              << " (" << width << "x" << height << ")\n";
}

GlfwWindow::~GlfwWindow()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
    std::cout << "[GlfwWindow] Window destroyed.\n";
}

bool GlfwWindow::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void GlfwWindow::pollEvents() const
{
    glfwPollEvents();
}

} // namespace giraphics
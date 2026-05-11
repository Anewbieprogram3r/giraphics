#include <Engine/Graphics/Window/Glfw/GlfwWindow.hpp>

#include <stdexcept>

namespace giraphics {

GlfwWindow::GlfwWindow(int w, int h, std::string name) : m_Width{w}, m_Height{h}, m_WindowName{name} {
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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_GlfwWindow = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(m_GlfwWindow, this);
  glfwSetFramebufferSizeCallback(m_GlfwWindow, framebufferResizeCallback);

  //// Set GLFW callbacks
  //glfwSetCursorPosCallback(m_GlfwWindow, MouseCallback);
  //glfwSetMouseButtonCallback(m_GlfwWindow, MouseButtonCallback);
  //glfwSetScrollCallback(m_GlfwWindow, ScrollCallback);
  //glfwSetKeyCallback(m_GlfwWindow, KeyboardCallback);

  //// Example: Register handlers
  //eventBus.RegisterMouseHandler([](const MouseEvent& event) {
  //    if (event.type == MouseEventType::MOUSE_MOVE) {
  //        std::cout << "Mouse moved to (" << event.x << ", " << event.y << ")\n";
  //    }
  //    else if (event.type == MouseEventType::MOUSE_SCROLL) {
  //        std::cout << "Mouse scrolled by " << event.scrollDelta << "\n";
  //    }
  //    });

  //eventBus.RegisterKeyboardHandler([](const KeyboardEvent& event) {
  //    if (event.type == KeyboardEventType::KEY_DOWN) {
  //        std::cout << "Key pressed: " << static_cast<int>(event.key) << "\n";
  //    }
  //    });
}

std::vector<const char*> GlfwWindow::getExtensions()
{
    uint32_t count = 0;
    const char** extensions;
    extensions = glfwGetRequiredInstanceExtensions(&count);

    // The std::vector will be moved automatically due to Return Value Optimization(RVO) and /or move semantics.
    return std::vector<const char*> (extensions, extensions + count);
}

void GlfwWindow::framebufferResizeCallback(GLFWwindow* p_window, int p_width, int p_height) {
  auto window = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(p_window));
  window->m_IsWindowResized = true;
  window->m_Width = p_width;
  window->m_Height = p_height;
}

}  // namespace giraphics

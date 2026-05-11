#include <Engine/Graphics/Window.hpp>

#include <Engine/Graphics/Window/Glfw/GlfwWindow.hpp>

giraphics::Window::Window(std::string name, int width, int height)
    : m_GlfwWindow(std::make_unique<GlfwWindow>(width, height, name))
{
}

giraphics::Window::~Window()
{
}

//void giraphics::Window::createSurface(void* instance, void* surface)
//{
//    m_GlfwWindow->createWindowSurface((VkInstance) instance, (VkSurfaceKHR *) surface);
//}

bool giraphics::Window::shouldClose()
{
    return m_GlfwWindow->shouldClose();
}

void giraphics::Window::pollEvents()
{
    return m_GlfwWindow->pollEvents();
}

uint32_t giraphics::Window::width()
{
    return m_GlfwWindow->getWidth();
}

uint32_t giraphics::Window::height()
{
    return m_GlfwWindow->getHeight();
}

bool giraphics::Window::wasWindowResized()
{
    return m_GlfwWindow->wasWindowResized();
}

void giraphics::Window::resetWindowResizedFlag()
{
    m_GlfwWindow->resetWindowResizedFlag();
}

void giraphics::Window::waitForEvents()
{
    m_GlfwWindow->waitForEvents();
}

std::vector<const char*> giraphics::Window::getExtensions()
{
    return m_GlfwWindow->getExtensions();
}


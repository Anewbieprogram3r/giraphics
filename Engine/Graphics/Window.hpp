#pragma once

#include <string>
#include <vector>
#include <memory>

namespace giraphics {

class GlfwWindow;
class Input;
class Window {
 public:
     Window(std::string name, int width = 800, int height = 600);
     ~Window();

     Window(const Window&) = delete;
     Window&operator=(const Window&) = delete;

     void createWindowSurface(void* instance, void* surface);
     bool shouldClose();
     void pollEvents();
     uint32_t width();
     uint32_t height();
     bool wasWindowResized();
     void resetWindowResizedFlag();
     void waitForEvents();
     std::vector<const char*> getExtensions();
     void setInput(Input& input);
     Input* input() const;
     void* glfwWindow();

private:
     std::unique_ptr<GlfwWindow> m_GlfwWindow;
};

}  // namespace giraphics
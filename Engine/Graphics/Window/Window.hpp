#pragma once

#include <string>
namespace giraphics {
    class Window {
    public:
        Window(const std::string& title, int width, int height)
            : m_title(title), m_width(width), m_height(height) {}
        virtual ~Window() = default;
        virtual bool shouldClose() const = 0;
        virtual void pollEvents() const = 0;
        int width() const { return m_width; }
        int height() const { return m_height; }
        std::string title() const { return m_title; }
        protected:
            std::string m_title;
            int m_width;
            int m_height;
    };
}